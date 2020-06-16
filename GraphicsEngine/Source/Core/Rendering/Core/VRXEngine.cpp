#include "VRXEngine.h"
#include "RHI\Shader.h"
#include "FrameBuffer.h"
#include "..\Shaders\VRX\Shader_VRRResolve.h"
#include "RHI\RHITimeManager.h"
#include "..\Shaders\Shader_Pair.h"
#include "SceneRenderer.h"
#include "Screen.h"
#include "RHI\RHITexture.h"
#include "..\Utils\UAVFormatConverter.h"
#include "Core\Utils\StringUtil.h"
#include "Rendering\Shaders\GlobalShaderLibrary.h"

REGISTER_SHADER_PS_ONEARG(VRRWriteStencil16,"VRX/VRRWriteStencil", ShaderProgramBase::Shader_Define("VRS_TILE_SIZE", "16"));
REGISTER_SHADER_PS_ONEARG(VRRWriteStencil8, "VRX/VRRWriteStencil", ShaderProgramBase::Shader_Define("VRS_TILE_SIZE", "8"));
REGISTER_SHADER_CS(Classify, "VRX/VRR_Classifycs");
REGISTER_SHADER_CS(LaunchJobs, "VRX/VRR_LaunchJobs");

VRXEngine* VRXEngine::Instance = nullptr;
VRXEngine::VRXEngine()
{
	if (RHI::GetRenderSettings()->GetVRXSettings().UseVRR(RHI::GetDefaultDevice()))
	{
		StencilWriteShader = new Shader_Pair(RHI::GetDefaultDevice(), { "Deferred_LightingPass_vs" ,"VRX/VRRWriteStencil" }, { EShaderType::SHADER_VERTEX,EShaderType::SHADER_FRAGMENT },
			{ ShaderProgramBase::Shader_Define("VRS_TILE_SIZE", std::to_string(RHI::GetDefaultDevice()->GetCaps().VRSTileSize)) });
		ResolvePS = new Shader_Pair(RHI::GetDefaultDevice(), { "Deferred_LightingPass_vs","VRX/VRRResolve_PS" }, { EShaderType::SHADER_VERTEX, EShaderType::SHADER_FRAGMENT });
		//ResolvePS = SHADER_resolve;
		VRRClassifyShader = new Shader_Pair(RHI::GetDefaultDevice(), { "VRX/VRR_Classifycs" }, { EShaderType::SHADER_COMPUTE });
		VRRLaunchShader = new Shader_Pair(RHI::GetDefaultDevice(), { "VRX/VRR_LaunchJobs" }, { EShaderType::SHADER_COMPUTE });
		TileData = RHI::CreateRHIBuffer(ERHIBufferType::GPU);
		RHIBufferDesc d;
		d.AllowUnorderedAccess = true;
		d.Accesstype = EBufferAccessType::GPUOnly;
		d.Stride = 8;
		d.ElementCount = 1;
		TileData->CreateBuffer(d);

		VARTileList = RHI::CreateRHIBuffer(ERHIBufferType::GPU);
		d.Stride = sizeof(uint) * 4;
		d.ElementCount = Screen::GetWindowRes().x *Screen::GetWindowRes().y;
		VARTileList->CreateBuffer(d);

		IndirectCommandBuffer = RHI::CreateRHIBuffer(ERHIBufferType::GPU);
		d.Stride = sizeof(IndirectDispatchArgs);
		d.ElementCount = 1;
		d.Accesstype = EBufferAccessType::GPUOnly;
		d.UseForExecuteIndirect = true;
		IndirectCommandBuffer->CreateBuffer(d);
		IndirectDispatchArgs Data;
		Data.ThreadGroupCountY = 1;
		Data.ThreadGroupCountZ = 1;
		Data.ThreadGroupCountX = 1;
		//	IndirectCommandBuffer->UpdateBufferData(&Data, sizeof(IndirectDispatchArgs), EBufferResourceState::UnorderedAccess);

			//debug

		VRXDebugShader = new Shader_Pair(RHI::GetDefaultDevice(), { "VRX/VRXDebug" }, { EShaderType::SHADER_COMPUTE },
			{ ShaderProgramBase::Shader_Define("VRS_TILE_SIZE", std::to_string(RHI::GetDefaultDevice()->GetCaps().VRSTileSize)) });
		ReadBackBuffer = RHI::CreateRHIBuffer(ERHIBufferType::ReadBack);
		DebugDataBuffer = RHI::CreateRHIBuffer(ERHIBufferType::GPU);
		RHIBufferDesc Desc = {};
		Desc.ElementCount = 1;
		Desc.Stride = sizeof(int);
		Desc.AllowUnorderedAccess = true;
		DebugDataBuffer->CreateBuffer(Desc);
		Desc.UseForExecuteIndirect = false;
		Desc.AllowUnorderedAccess = false;
		Desc.UseForReadBack = true;
		Desc.Accesstype = EBufferAccessType::ReadBack;
		ReadBackBuffer->CreateBuffer(Desc);
	}
}


VRXEngine::~VRXEngine()
{}

VRXEngine * VRXEngine::Get()
{
	if (Instance == nullptr)
	{
		Instance = new VRXEngine();
	}
	return Instance;
}

void VRXEngine::ResolveVRRFramebuffer_PS(RHICommandList* list, FrameBuffer* Target, RHITexture* ShadingImage)
{
#if USEPS_VRR
	DECALRE_SCOPEDGPUCOUNTER(list, "VRX Resolve PS");
	VRXEngine::Get()->TempTexture->SetState(list, EResourceState::PixelShader);
	//ensure(list->IsComputeList());
	RHIPipeLineStateDesc Desc = RHIPipeLineStateDesc::CreateDefault(Get()->ResolvePS, Target);
	Desc.DepthStencilState.DepthEnable = false;
	Desc.Cull = false;
	Desc.RasterizerState.Cull = false;
	list->SetPipelineStateDesc(Desc);
	if (ShadingImage != nullptr)
	{
		list->SetTexture2(ShadingImage, "RateImage");
	}
	list->SetTexture2(VRXEngine::Get()->TempTexture, "TempBuffer");
	ShaderCompiler::GetShader<Shader_VRRResolve>()->BindBuffer(list);
	glm::ivec2 Resoloution = Screen::GetScaledRes();
	list->SetRootConstant("ResData", 2, &Resoloution);
	list->BeginRenderPass(RHIRenderPassDesc(Target));
	SceneRenderer::DrawScreenQuad(list);
	list->EndRenderPass();
#endif
}

void VRXEngine::ResolveVRRFramebuffer(RHICommandList* list, FrameBuffer* Target, RHITexture* ShadingImage)
{
#if USEPS_VRR
	if (!RHI::GetRenderSettings()->GetVRXSettings().UseVRX())
	{
		return;
	}
	RenderDebug(list, Target, ShadingImage);
	return;
#endif
	if (!RenderSettings::GetVRXSettings().UseVRR())
	{
		if (RenderSettings::GetVRXSettings().UseVRS(list->GetDevice()))
		{
			RenderDebug(list, Target, ShadingImage);
		}
		return;
	}
	if (Target->GetDescription().VarRateSettings.BufferMode != FrameBufferVariableRateSettings::VRR)
	{
		return;
	}
	if (!RHI::GetRenderSettings()->GetVRXSettings().VRXActive)
	{
		return;
	}
	DECALRE_SCOPEDGPUCOUNTER(list, "VRX Resolve");
	if (list->IsGraphicsList())
	{
		ResolveVRRFramebuffer_PS(list, Target, ShadingImage);
		return;
	}
	ensure(list->IsComputeList());

	RHIPipeLineStateDesc Desc = RHIPipeLineStateDesc::CreateDefault(Get()->VRRClassifyShader);
	{
		DECALRE_SCOPEDGPUCOUNTER(list, "VRX Resolve.Classify");
		list->SetPipelineStateDesc(Desc);
		list->SetTexture2(ShadingImage, "RateImage");
		Get()->TileData->SetBufferState(list, EBufferResourceState::UnorderedAccess);
		Get()->VARTileList->SetBufferState(list, EBufferResourceState::UnorderedAccess);
		list->SetUAV(Get()->TileData, "TileData");
		list->SetUAV(Get()->VARTileList, "TileList_VAR");
		list->DispatchSized(ShadingImage->GetDescription().Width, ShadingImage->GetDescription().Height, 1);
		list->UAVBarrier(Get()->TileData);
		list->UAVBarrier(Get()->VARTileList);

		Get()->IndirectCommandBuffer->SetBufferState(list, EBufferResourceState::UnorderedAccess);
		list->SetPipelineStateDesc(RHIPipeLineStateDesc::CreateDefault(Get()->VRRLaunchShader));
		list->SetUAV(Get()->IndirectCommandBuffer, "IndirectCommandBuffer_VAR");
		list->SetUAV(Get()->TileData, "TileData");
		list->Dispatch(1, 1, 1);
		list->UAVBarrier(Get()->IndirectCommandBuffer);
		list->UAVBarrier(Get()->TileData);
		Get()->IndirectCommandBuffer->SetBufferState(list, EBufferResourceState::IndirectArgs);
	}
#if 0
	if (!list->GetDevice()->GetCaps().SupportTypedUAVLoads)
	{
		DECALRE_SCOPEDGPUCOUNTER(list, "VRX Resolve.Copy Step");
		if (Get()->TempResolveSpace == nullptr || Get()->TempResolveSpace->GetDescription().Width != Target->GetWidth() || Get()->TempResolveSpace->GetDescription().Height != Target->GetHeight())
		{
			if (Get()->TempResolveSpace)
			{
				EnqueueSafeRHIRelease(Get()->TempResolveSpace);
			}
			Get()->TempResolveSpace = RHI::GetRHIClass()->CreateTexture2();
			RHITextureDesc2 TmpDesc;
			TmpDesc.Depth = 1;
			TmpDesc.Width = Target->GetWidth();
			TmpDesc.Height = Target->GetHeight();
			TmpDesc.Format = eTEXTURE_FORMAT::FORMAT_R16G16_TYPELESS;// Target->GetDescription().RTFormats[0];
			TmpDesc.AllowUnorderedAccess = true;
			Get()->TempResolveSpace->Create(TmpDesc, list->GetDevice());
		}

		list->CopyResource(Target->GetDescription().RenderTargets[0], Get()->TempResolveSpace);
		Get()->TempResolveSpace->SetState(list, EResourceState::Non_PixelShader);

	}
#else
	Target->SetResourceState(list, EResourceState::UAV);
	UAVFormatConverter::UnPackToTmpResource(&Get()->TempResolveSpace, list, Target->GetDescription().RenderTargets[0]);
#endif
	{
		DECALRE_SCOPEDGPUCOUNTER(list, "VRX Resolve.Varable Write");
		Desc = RHIPipeLineStateDesc::CreateDefault(GlobalShaderLibrary::VRXResolveShader->Get(list->GetDevice()));
		list->SetPipelineStateDesc(Desc);
		RHICommandSignitureDescription SigDesc;
		INDIRECT_ARGUMENT_DESC DispatchDesc;
		DispatchDesc.Type = INDIRECT_ARGUMENT_TYPE::INDIRECT_ARGUMENT_TYPE_DISPATCH;
		SigDesc.ArgumentDescs.push_back(DispatchDesc);
		SigDesc.IsCompute = true;
		SigDesc.CommandBufferStide = sizeof(IndirectDispatchArgs);
		list->SetCommandSigniture(SigDesc);

		if (!list->GetDevice()->GetCaps().SupportTypedUAVLoads)
		{
			RHIViewDesc D = RHIViewDesc::DefaultUAV();
			D.UseResourceFormat = false;
			D.Format = ETextureFormat::FORMAT_R32_UINT;
			list->SetUAV(Get()->TempResolveSpace, list->GetCurrnetPSO()->GetDesc().ShaderInUse->GetSlotForName("DstTexture"), D);
		}
		else
		{
			list->SetUAV(Target, "DstTexture");
		}
		/*if (!list->GetDevice()->GetCaps().SupportTypedUAVLoads)
		{
			list->SetTexture2(Get()->TempResolveSpace, "SrcTexture");
		}*/

		//list->SetTexture2(ShadingImage, "RateImage");
		//
		//Get()->TileData->SetBufferState(list, EBufferResourceState::Non_PixelShader);
		Get()->VARTileList->SetBufferState(list, EBufferResourceState::Non_PixelShader);
		list->SetBuffer(Get()->VARTileList, "TileList");
		ShaderCompiler::GetShader<Shader_VRRResolve>()->BindBuffer(list);
		list->SetUAV(Get()->TileData, "TileData");
		list->ExecuteIndirect(1, Get()->IndirectCommandBuffer, 0, nullptr, 0);
		list->UAVBarrier(Target);
	}
	UAVFormatConverter::PackBacktoResource(Get()->TempResolveSpace, list, Target->GetDescription().RenderTargets[0]);
	RenderDebug(list, Target, ShadingImage);
}

void VRXEngine::RenderDebug(RHICommandList* list, FrameBuffer* Target, RHITexture* ShadingImage)
{
	//return;
	if (ShaderCompiler::GetShader<Shader_VRRResolve>()->IsDebugActive())
	{
		list->SetPipelineStateDesc(RHIPipeLineStateDesc::CreateDefault(Get()->VRXDebugShader));
		ShaderCompiler::GetShader<Shader_VRRResolve>()->BindBuffer(list);
		list->SetTexture2(ShadingImage, "RateImage");
		list->SetUAV(Target, "DstTexture");
		Get()->DebugDataBuffer->SetBufferState(list, EBufferResourceState::UnorderedAccess);
		list->SetUAV(Get()->DebugDataBuffer, "PixelCount");
		list->DispatchSized(Target->GetWidth(), Target->GetHeight(), 1);
		list->UAVBarrier(Target);
		list->UAVBarrier(Get()->DebugDataBuffer);
		Get()->DebugDataBuffer->SetBufferState(list, EBufferResourceState::CopySrc);
		list->CopyResource(Get()->DebugDataBuffer, Get()->ReadBackBuffer);
	}
	int* data = (int*)Get()->ReadBackBuffer->MapReadBack();
	int TotalPixels = Screen::GetScaledWidth()*Screen::GetScaledHeight();
	std::string out = " Pixel Shaded: " + std::to_string(*data) + " (" + StringUtils::ToString((*data / (float)TotalPixels)) + ")";
	Log::Get()->LogTextToScreen(out, 0);

}

void VRXEngine::SetVRXShadingRateImage(RHICommandList * List, RHITexture * Target)
{
	if (List->GetCurrnetPSO() != nullptr && List->GetCurrnetPSO()->GetDesc().ShaderInUse != nullptr)
	{
		const std::string TextureName = "VRSTexture";
		if (List->GetCurrnetPSO()->GetDesc().ShaderInUse->FindParam(TextureName) != nullptr)
		{
			//			List->SetTexture2(List->GetShadingRateImage(), TextureName);
		}
	}
}

void VRXEngine::SetupVRRShader(Shader* S, DeviceContext* device)
{
	if (RenderSettings::GetVRXSettings().UseVRR())
	{
		S->GetShaderProgram()->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("SUPPORT_VRR", "1"));
		S->GetShaderProgram()->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("SHADER_SUPPORT_VRR", "1"));
		S->GetShaderProgram()->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("VRS_TILE_SIZE", std::to_string(device->GetCaps().VRSTileSize)));
	}
}

void VRXEngine::AddVRRToRS(std::vector<ShaderParameter>& S, int lastindex /*= 0*/)
{
	if (!RenderSettings::GetVRXSettings().UseVRR())
	{
		return;
	}
	ShaderParameter Sp = ShaderParameter(ShaderParamType::SRV, lastindex, 66);
	Sp.Name = "VRSImage";
	S.push_back(Sp);
	Sp = ShaderParameter(ShaderParamType::UAV, lastindex + 1, 66);
	Sp.Name = "VRSImage";
	S.push_back(Sp);
}

void VRXEngine::WriteVRRStencil(RHICommandList* List, FrameBuffer* MainBuffer)
{
	if (RHI::GetRenderSettings()->GetVRXSettings().UseVRR() && MainBuffer != nullptr && MainBuffer->GetDescription().HasStencil())
	{
		DECALRE_SCOPEDGPUCOUNTER(List, "VRX Stencil Write");
		//write the depth stencil
		RHIPipeLineStateDesc desc = RHIPipeLineStateDesc();
		desc.InitOLD(false, false, false);
		desc.ShaderInUse = Get()->StencilWriteShader;
		desc.RenderTargetDesc = MainBuffer->GetPiplineRenderDesc();
		desc.DepthStencilState.StencilEnable = true;
		desc.DepthStencilState.BackFace.StencilFunc = COMPARISON_FUNC_EQUAL;
		desc.DepthStencilState.FrontFace.StencilFunc = COMPARISON_FUNC_EQUAL;
		//looks like discard acts as a stencil pass
		desc.DepthStencilState.FrontFace.StencilPassOp = STENCIL_OP_INCR;
		desc.DepthStencilState.BackFace.StencilPassOp = STENCIL_OP_INCR;
		List->SetPipelineStateDesc(desc);
		List->SetTexture2(List->GetShadingRateImage(), "RateImage");
		glm::ivec2 Resoloution = Screen::GetScaledRes();
		List->SetRootConstant("ResData", 2, &Resoloution);
		RHIRenderPassDesc D = RHIRenderPassDesc(MainBuffer, ERenderPassLoadOp::Clear);
		List->BeginRenderPass(D);
		SceneRenderer::DrawScreenQuad(List);
		List->EndRenderPass();
	}
}


