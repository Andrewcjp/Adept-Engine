#include "VRXShadingRateNode.h"
#include "Rendering/Core/FrameBuffer.h"
#include "Rendering/Core/Screen.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "Rendering/Shaders/Shader_Pair.h"
#include "RHI/RHITimeManager.h"

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
	{
		DECALRE_SCOPEDGPUCOUNTER(List, "VRX Image");
		RHIPipeLineStateDesc PSODesc = RHIPipeLineStateDesc::CreateDefault(Shader);
		List->SetPipelineStateDesc(PSODesc);
		FrameBuffer* UAV = GetFrameBufferFromInput(0);
		UAV->SetResourceState(List, EResourceState::UAV);
		FrameBuffer* gBuffer = GetFrameBufferFromInput(1);
		//List->SetFrameBufferTexture(gBuffer, "GBuffer_Pos", 0);
		List->SetFrameBufferTexture(GetFrameBufferFromInput(2), "ShadowMask");
		//#VRX todo: use inputs from scene for this (Gbuffer, PreZ pass)
		List->SetUAV(UAV, "RateData");
		glm::ivec2 Resoloution = Screen::GetScaledRes();
		List->SetRootConstant("ResData", 2, &Resoloution);
		List->DispatchSized(UAV->GetWidth(), UAV->GetHeight(), 1);
		List->UAVBarrier(UAV);
		UAV->SetResourceState(List, EResourceState::Non_PixelShader);
	}
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
	AddResourceInput(EStorageType::Framebuffer, EResourceState::Non_PixelShader, StorageFormats::GBufferData, "Gbuffer");
	AddResourceInput(EStorageType::Framebuffer, EResourceState::Non_PixelShader, StorageFormats::PreSampleShadowData, "ShadowMask");
	AddResourceOutput(EStorageType::Framebuffer, EResourceState::ComputeUse, StorageFormats::ShadingImage, "Shading Rate Image");
}

void VRXShadingRateNode::OnSetupNode()
{
	List = RHI::CreateCommandList(ECommandListType::Compute, Context);
	Shader = new Shader_Pair(Context, { "VRX/VRXGScreen" }, { EShaderType::SHADER_COMPUTE }, { ShaderProgramBase::Shader_Define("VRS_TILE_SIZE", std::to_string(Context->GetCaps().VRSTileSize)) });
}
