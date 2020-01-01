#include "VRXShadingRateNode.h"
#include "Rendering/Core/FrameBuffer.h"
#include "Rendering/Core/Screen.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "Rendering/Shaders/Shader_Pair.h"
#include "RHI/RHITimeManager.h"


VRXShadingRateNode::VRXShadingRateNode()
{
	NodeEngineType = ECommandListType::Compute;
	OnNodeSettingChange();
}


VRXShadingRateNode::~VRXShadingRateNode()
{}

void VRXShadingRateNode::OnExecute()
{
	FLAT_COMPUTE_START(Context);
	List = Context->GetListPool()->GetCMDList(ECommandListType::Compute);
	{
		SetBeginStates(List);
		DECALRE_SCOPEDGPUCOUNTER(List, "VRX Image");
		RHIPipeLineStateDesc PSODesc = RHIPipeLineStateDesc::CreateDefault(Shader);
		List->SetPipelineStateDesc(PSODesc);
		FrameBuffer* UAV = GetFrameBufferFromInput(0);
		FrameBuffer* gBuffer = GetFrameBufferFromInput(1);
		//List->SetFrameBufferTexture(gBuffer, "GBuffer_Pos", 0);
		List->SetFrameBufferTexture(GetFrameBufferFromInput(2), "ShadowMask");
		//#VRX todo: use inputs from scene for this (Gbuffer, PreZ pass)
		List->SetUAV(UAV, "RateData");
		glm::ivec2 Resoloution = Screen::GetScaledRes();
		List->SetRootConstant("ResData", 2, &Resoloution);
		List->DispatchSized(UAV->GetWidth(), UAV->GetHeight(), 1);
		List->UAVBarrier(UAV);
		SetEndStates(List);
	}
	Context->GetListPool()->Flush();
	FLAT_COMPUTE_END(Context);
}

bool VRXShadingRateNode::IsNodeSupported(const RenderSettings& settings)
{
	return settings.GetVRXSettings().UseVRR() ;
}

void VRXShadingRateNode::OnNodeSettingChange()
{
	AddResourceInput(EStorageType::Framebuffer, EResourceState::UAV, StorageFormats::ShadingImage, "Shading Rate Image");
	AddResourceInput(EStorageType::Framebuffer, EResourceState::Non_PixelShader, StorageFormats::GBufferData, "Gbuffer");
	AddResourceInput(EStorageType::Framebuffer, EResourceState::Non_PixelShader, StorageFormats::PreSampleShadowData, "ShadowMask");
	AddResourceOutput(EStorageType::Framebuffer, EResourceState::Non_PixelShader, StorageFormats::ShadingImage, "Shading Rate Image");
	LinkThough(0);
}

void VRXShadingRateNode::OnSetupNode()
{
	List = RHI::CreateCommandList(ECommandListType::Compute, Context);
	Shader = new Shader_Pair(Context, { "VRX/VRXGScreen" }, { EShaderType::SHADER_COMPUTE }, { ShaderProgramBase::Shader_Define("VRS_TILE_SIZE", std::to_string(Context->GetCaps().VRSTileSize)) });
}
