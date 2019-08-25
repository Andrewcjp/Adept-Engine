#include "VRXEngine.h"
#include "RHI\Shader.h"
#include "FrameBuffer.h"
#include "..\Shaders\VRX\Shader_VRRResolve.h"
#include "..\Shaders\VRX\Shader_VRSResolve.h"


VRXEngine::VRXEngine()
{}


VRXEngine::~VRXEngine()
{}

VRXEngine * VRXEngine::Get()
{
	return nullptr;
}

void VRXEngine::ResolveVRRFramebuffer(RHICommandList* list, FrameBuffer* Target)
{
	if (!RenderSettings::GetVRXSettings().EnableVRR)
	{
		return;
	}
	if (Target->GetDescription().VarRateSettings.BufferMode != FrameBufferVariableRateSettings::VRR)
	{
		return;
	}
	//#VRX: todo
	ensure(list->IsComputeList());
	RHIPipeLineStateDesc Desc = RHIPipeLineStateDesc::CreateDefault(ShaderComplier::GetShader<Shader_VRSResolve>());
	list->SetPipelineStateDesc(Desc);
	list->SetUAV(Target->GetUAV(), "DstTexture");
	ShaderComplier::GetShader<Shader_VRSResolve>()->BindBuffer(list);
	list->Dispatch(Target->GetWidth() / 4, Target->GetHeight() / 4, 1);
	list->UAVBarrier(Target->GetUAV());
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
	ensure(list->IsComputeList());
	RHIPipeLineStateDesc Desc = RHIPipeLineStateDesc::CreateDefault(ShaderComplier::GetShader<Shader_VRSResolve>());
	list->SetPipelineStateDesc(Desc);
	list->SetUAV(Target->GetUAV(), "DstTexture");
	ShaderComplier::GetShader<Shader_VRSResolve>()->BindBuffer(list);
	list->Dispatch(glm::ceil(Target->GetWidth() / 4), glm::round(Target->GetHeight() / 4), 1);
	list->UAVBarrier(Target->GetUAV());
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

void VRXEngine::SetVRXShadingRateImage(RHICommandList * List, FrameBuffer * Target)
{
	//#VRX: todo

}

void VRXEngine::SetupVRRShader(Shader * S)
{
	if (RenderSettings::GetVRXSettings().EnableVRR)
	{
		S->GetShaderProgram()->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("SUPPORT_VRR", "1"));
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

void VRXEngine::AddVRSToRS(std::vector<ShaderParameter>& S, int lastindex)
{
	if (!RenderSettings::GetVRXSettings().EnableVRS)
	{
		return;
	}
	ShaderParameter Sp = ShaderParameter(ShaderParamType::SRV, lastindex, 66);
	Sp.Name = "VRSImage";
	S.push_back(Sp);
}
