#include "VRXShadingRateNode.h"
#include "Rendering/Core/FrameBuffer.h"
#include "Rendering/Core/Screen.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "Rendering/Shaders/Shader_Pair.h"
#include "RHI/RHITimeManager.h"
#include "Core/Input/Input.h"
#include "Core/Maths/Math.h"
#include "RHI/RHITexture.h"
REGISTER_SHADER_CS_ONEARG(vrxGen16, "VRX/VRXGScreen", ShaderProgramBase::Shader_Define("VRS_TILE_SIZE", std::to_string(16)));
REGISTER_SHADER_CS_ONEARG(vrxGen8, "VRX/VRXGScreen", ShaderProgramBase::Shader_Define("VRS_TILE_SIZE", std::to_string(8)));

static ConsoleVariable Threshold("VRR.Geo", 0.02f, ECVarType::ConsoleAndLaunch);
static ConsoleVariable FullResThreshold("VRR.Full", 1, ECVarType::ConsoleAndLaunch);
static ConsoleVariable HalfResThreshold("VRR.Half", 0, ECVarType::ConsoleAndLaunch);
VRXShadingRateNode::VRXShadingRateNode()
{
	NodeEngineType = ECommandListType::Compute;
	OnNodeSettingChange();
}


VRXShadingRateNode::~VRXShadingRateNode()
{}

void VRXShadingRateNode::OnExecute()
{
	if (Input::GetKey(']'))
	{
		FullResThreshold.SetValue(FullResThreshold.GetIntValue() + 1);
		HalfResThreshold.SetValue(FullResThreshold.GetIntValue() / 2);
	}
	if (Input::GetKey('['))
	{
		FullResThreshold.SetValue(FullResThreshold.GetIntValue() - 1);
		HalfResThreshold.SetValue(FullResThreshold.GetIntValue() / 2);
	}
	if (Input::GetKey('#'))
	{
		Threshold.SetValueF(Threshold.GetFloatValue() + 0.1f);
	}
	if (Input::GetKey('\''))
	{
		Threshold.SetValueF(Threshold.GetFloatValue() - 0.1f);
	}
	FullResThreshold.SetValue(Math::Max(FullResThreshold.GetIntValue(), 0));
	HalfResThreshold.SetValue(Math::Max(FullResThreshold.GetIntValue() / 2, 0));
	FLAT_COMPUTE_START(Context);
	List = Context->GetListPool()->GetCMDList(ECommandListType::Compute);
	{
		SetBeginStates(List);
		Data.GeoThreshold = Threshold.GetFloatValue();
		Data.FullResTheshold = FullResThreshold.GetIntValue();
		Data.HalfResTheshold = HalfResThreshold.GetIntValue();
		Data.DebugedgeCount = RHI::GetRenderSettings()->GetDebugRenderMode() == ERenderDebugOutput::Scene_EdgeDetectCount;
		Data.WriteZeroImage = !RHI::GetRenderSettings()->GetVRXSettings().VRXActive;
		Constant->UpdateConstantBuffer(&Data);
		DECALRE_SCOPEDGPUCOUNTER(List, "VRX Image");
		RHIPipeLineStateDesc PSODesc = RHIPipeLineStateDesc::CreateDefault(Shader);
		List->SetPipelineStateDesc(PSODesc);
		FrameBuffer* UAV = GetFrameBufferFromInput(0);
		FrameBuffer* gBuffer = GetFrameBufferFromInput(1);
		List->SetFrameBufferTexture(gBuffer, "GBuffer_Tex");
	//	gBuffer->GetDepthStencil()->SetState(List, EResourceState::PixelShader);
		//List->SetTexture2(gBuffer->GetDepthStencil(), "GBuffer_Tex");
		//List->SetFrameBufferTexture(GetFrameBufferFromInput(2), "ShadowMask");
		//List->ClearUAVUint(UAV);
		//#VRX todo: use inputs from scene for this (Gbuffer, PreZ pass)
		List->SetConstantBufferView(Constant, 0, "GData");
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
	return settings.GetVRXSettings().UseVRX();
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
	Constant = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
	Constant->CreateConstantBuffer(sizeof(Data), 1);
}
