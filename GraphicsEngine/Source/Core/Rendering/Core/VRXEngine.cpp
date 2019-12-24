#include "VRXEngine.h"
#include "RHI\Shader.h"
#include "FrameBuffer.h"
#include "..\Shaders\VRX\Shader_VRRResolve.h"
#include "..\Shaders\VRX\Shader_VRSResolve.h"
#include "RHI\RHITimeManager.h"
#include "..\Shaders\Shader_Pair.h"
#include "SceneRenderer.h"


VRXEngine::VRXEngine()
{}


VRXEngine::~VRXEngine()
{}

VRXEngine * VRXEngine::Get()
{
	return nullptr;
}

void VRXEngine::ResolveVRRFramebuffer_PS(RHICommandList* list, FrameBuffer* Target, RHITexture* ShadingImage)
{
	if (Get()->ResolvePS == nullptr)
	{
		Get()->ResolvePS = new Shader_Pair(list->GetDevice(), { "Deferred_LightingPass_vs","VRX/VRRResolve_PS" }, { EShaderType::SHADER_VERTEX, EShaderType::SHADER_FRAGMENT });
	}
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
	if (!RenderSettings::GetVRXSettings().EnableVRR)
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
	const int TileSize = 16;
	list->DispatchSized(Target->GetWidth(), Target->GetHeight(), 1);
	list->UAVBarrier(Target);
}

void VRXEngine::ResolveVRSFramebuffer(RHICommandList* list, FrameBuffer* Target)
{
	if (!RenderSettings::GetVRXSettings().EnableVRS)
	{
		return;
	}
	if (Target->GetDescription().VarRateSettings.BufferMode != FrameBufferVariableRateSettings::VRS)
	{
		return;
	}
	DECALRE_SCOPEDGPUCOUNTER(list, "VRS Resolve");
	ensure(list->IsComputeList());
	RHIPipeLineStateDesc Desc = RHIPipeLineStateDesc::CreateDefault(ShaderComplier::GetShader<Shader_VRSResolve>());
	list->SetPipelineStateDesc(Desc);
	list->SetUAV(Target, "DstTexture");
	ShaderComplier::GetShader<Shader_VRSResolve>()->BindBuffer(list);
	list->Dispatch(glm::ceil(Target->GetWidth() / 4), Target->GetHeight() / 4, 1);
	list->UAVBarrier(Target);
}

void VRXEngine::SetVRSShadingRate(RHICommandList * List, VRS_SHADING_RATE::type Rate)
{
	//#VRX: todo
}

void VRXEngine::SetVRRShadingRate(RHICommandList * List, int FactorIndex)
{
	//#VRX: todo
	int slot = 13;// List->GetCurrnetPSO()->GetDesc().ShaderInUse->GetSlotForName("VRRData");
	List->SetSingleRootConstant(slot, FactorIndex);
}

void VRXEngine::SetVRXShadingRateImage(RHICommandList * List, RHITexture * Target)
{
	//#VRX: todo

}

void VRXEngine::SetupVRRShader(Shader* S, DeviceContext* device)
{
	if (RenderSettings::GetVRXSettings().EnableVRR)
	{
		S->GetShaderProgram()->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("SUPPORT_VRR", "1"));
		S->GetShaderProgram()->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("VRS_TILE_SIZE", std::to_string(device->GetCaps().VRSTileSize)));
	}
}

void VRXEngine::AddVRRToRS(std::vector<ShaderParameter>& S, int lastindex /*= 0*/)
{
	if (!RenderSettings::GetVRXSettings().EnableVRR)
	{
		return;
	}
	ShaderParameter Sp = ShaderParameter(ShaderParamType::RootConstant, lastindex, 65);
	Sp.Name = "VRRData";
	S.push_back(Sp);
}

void VRXEngine::SetupVRSShader(Shader * S)
{
	if (RenderSettings::GetVRXSettings().EnableVRS)
	{
		S->GetShaderProgram()->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("SUPPORT_VRS", "1"));
	}
}

void VRXEngine::AddVRSToRS(std::vector<ShaderParameter>& S, uint64 lastindex /*= 0*/)
{
	if (!RenderSettings::GetVRXSettings().EnableVRS)
	{
		return;
	}
	ShaderParameter Sp = ShaderParameter(ShaderParamType::SRV, lastindex, 66);
	Sp.Name = "VRSImage";
	S.push_back(Sp);
}
