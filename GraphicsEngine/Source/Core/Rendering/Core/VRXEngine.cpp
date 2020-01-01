#include "VRXEngine.h"
#include "RHI\Shader.h"
#include "FrameBuffer.h"
#include "..\Shaders\VRX\Shader_VRRResolve.h"
#include "RHI\RHITimeManager.h"
#include "..\Shaders\Shader_Pair.h"
#include "SceneRenderer.h"
#include "Screen.h"
VRXEngine* VRXEngine::Instance = nullptr;

VRXEngine::VRXEngine()
{
	if (RHI::GetRenderSettings()->GetVRXSettings().UseVRR(RHI::GetDefaultDevice()))
	{
		StencilWriteShader = new Shader_Pair(RHI::GetDefaultDevice(), { "Deferred_LightingPass_vs" ,"VRX/VRRWriteStencil" }, { EShaderType::SHADER_VERTEX,EShaderType::SHADER_FRAGMENT },
			{ ShaderProgramBase::Shader_Define("VRS_TILE_SIZE", std::to_string(RHI::GetDefaultDevice()->GetCaps().VRSTileSize)) });
		ResolvePS = new Shader_Pair(RHI::GetDefaultDevice(), { "Deferred_LightingPass_vs","VRX/VRRResolve_PS" }, { EShaderType::SHADER_VERTEX, EShaderType::SHADER_FRAGMENT });
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
	ensure(list->IsComputeList());
	RHIPipeLineStateDesc Desc = RHIPipeLineStateDesc::CreateDefault(Get()->ResolvePS);
	Desc.DepthStencilState.DepthEnable = false;
	list->SetPipelineStateDesc(Desc);
	if (ShadingImage != nullptr)
	{
		list->SetTexture2(ShadingImage, "RateImage");
	}
	ShaderComplier::GetShader<Shader_VRRResolve>()->BindBuffer(list);
	list->BeginRenderPass(RHIRenderPassDesc(Target));
	SceneRenderer::DrawScreenQuad(list);
	list->EndRenderPass();
}

void VRXEngine::ResolveVRRFramebuffer(RHICommandList* list, FrameBuffer* Target, RHITexture* ShadingImage)
{
	if (!RenderSettings::GetVRXSettings().UseVRR())
	{
		return;
	}
	if (Target->GetDescription().VarRateSettings.BufferMode != FrameBufferVariableRateSettings::VRR)
	{
		return;
	}
	DECALRE_SCOPEDGPUCOUNTER(list, "VRR Resolve");
	if (list->IsGraphicsList())
	{
		ResolveVRRFramebuffer_PS(list, Target, ShadingImage);
		return;
	}
	ensure(list->IsComputeList());
	RHIPipeLineStateDesc Desc = RHIPipeLineStateDesc::CreateDefault(ShaderComplier::GetShader<Shader_VRRResolve>());
	list->SetPipelineStateDesc(Desc);
	list->SetUAV(Target, "DstTexture");
	if (ShadingImage != nullptr)
	{
		list->SetTexture2(ShadingImage, "RateImage");
	}
	ShaderComplier::GetShader<Shader_VRRResolve>()->BindBuffer(list);
	list->DispatchSized(Target->GetWidth(), Target->GetHeight(), 1);
	list->UAVBarrier(Target);
}

void VRXEngine::SetVRRShadingRate(RHICommandList * List, int FactorIndex)
{
	//#VRX: todo
}

void VRXEngine::SetVRXShadingRateImage(RHICommandList * List, RHITexture * Target)
{
	if (List->GetCurrnetPSO() != nullptr && List->GetCurrnetPSO()->GetDesc().ShaderInUse != nullptr)
	{
		const std::string TextureName = "VRSTexture";
		if (List->GetCurrnetPSO()->GetDesc().ShaderInUse->FindParam(TextureName) != nullptr)
		{
			List->SetTexture2(List->GetShadingRateImage(), TextureName);
		}
	}
}

void VRXEngine::SetupVRRShader(Shader* S, DeviceContext* device)
{
	if (RenderSettings::GetVRXSettings().UseVRR())
	{
		S->GetShaderProgram()->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("SUPPORT_VRR", "1"));
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
}

void VRXEngine::WriteVRRStencil(RHICommandList* List, FrameBuffer* MainBuffer)
{
	if (RHI::GetRenderSettings()->GetVRXSettings().UseVRR() && MainBuffer != nullptr && MainBuffer->GetDescription().HasStencil())
	{
		DECALRE_SCOPEDGPUCOUNTER(List, "VRR Stencil Write");
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


