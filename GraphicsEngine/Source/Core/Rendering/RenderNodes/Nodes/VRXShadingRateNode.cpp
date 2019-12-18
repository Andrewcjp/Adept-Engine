#include "VRXShadingRateNode.h"
#include "Rendering/Core/FrameBuffer.h"
#include "Rendering/Core/Screen.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "Rendering/Shaders/Shader_Pair.h"

VRXShadingRateNode::VRXShadingRateNode()
{
	OnNodeSettingChange();
}


VRXShadingRateNode::~VRXShadingRateNode()
{}

void VRXShadingRateNode::OnExecute()
{
	FLAT_COMPUTE_START(Context);
	List->ResetList();
	RHIPipeLineStateDesc PSODesc = RHIPipeLineStateDesc::CreateDefault(Shader);
	List->SetPipelineStateDesc(PSODesc);
	FrameBuffer* UAV = GetFrameBufferFromInput(0);
	UAV->SetResourceState(List, EResourceState::UAV);
	//#VRX todo: use inputs from scene for this (Gbuffer, PreZ pass)
	List->SetUAV(UAV, "RateData");
	glm::ivec2 Resoloution = Screen::GetScaledRes();
	List->SetRootConstant("ResData", 2, &Resoloution);
	List->Dispatch(UAV->GetWidth(), UAV->GetHeight(), 1);
	List->UAVBarrier(UAV);
	UAV->SetResourceState(List, EResourceState::Non_PixelShader);
	List->Execute();
	FLAT_COMPUTE_END(Context);
}

bool VRXShadingRateNode::IsNodeSupported(const RenderSettings& settings)
{
	return settings.GetVRXSettings().EnableVRR || settings.GetVRXSettings().EnableVRS;
}

void VRXShadingRateNode::OnNodeSettingChange()
{
	AddResourceInput(EStorageType::Framebuffer, EResourceState::ComputeUse, StorageFormats::ShadingImage, "Shading Rate Image");
	AddResourceOutput(EStorageType::Framebuffer, EResourceState::ComputeUse, StorageFormats::ShadingImage, "Shading Rate Image");
}

void VRXShadingRateNode::OnSetupNode()
{
	List = RHI::CreateCommandList(ECommandListType::Compute, Context);
	Shader = new Shader_Pair(Context, { "VRX/VRXGScreen" }, { EShaderType::SHADER_COMPUTE }, { ShaderProgramBase::Shader_Define("VRS_TILE_SIZE", std::to_string(Context->GetCaps().VRSTileSize)) });
}
