#include "RTFilterNode.h"
#include "Core/Assets/AssetManager.h"
#include "Rendering/Core/DynamicQualityEngine.h"
#include "Rendering/Core/FrameBuffer.h"
#include "Rendering/Core/Screen.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "Rendering/Shaders/Shader_Pair.h"

static ConsoleVariable DebugMode("RT.Noise.Debug", 0, ECVarType::ConsoleAndLaunch, true);
RTFilterNode::RTFilterNode()
{
	OnNodeSettingChange();
	NodeEngineType = ECommandListType::Compute;
	DebugMode.SetValue(2);
}

RTFilterNode::~RTFilterNode()
{}

void RTFilterNode::OnExecute()
{
	Params.Debugmode = DebugMode.GetIntValue();
	DenoiserData->UpdateConstantBuffer(&Params);
	FrameBuffer* Target = GetFrameBufferFromInput(0);
	RHICommandList* list = Context->GetListPool()->GetCMDList(ECommandListType::Compute);
	SetBeginStates(list);
	list->UAVBarrier(Target);
	if (Clear)
	{
		//list->ClearFrameBuffer(GetFrameBufferFromInput(2));
//		list->ClearFrameBuffer(GetFrameBufferFromInput(3));
		Clear = false;
	}
	list->SetPipelineStateDesc(RHIPipeLineStateDesc::CreateDefault(MergeShader));
	list->SetUAV(Target, "RTXUAV");
	list->SetFrameBufferTexture(GetFrameBufferFromInput(1), "VxIn");
	DynamicQualityEngine::Get()->BindRTBuffer(list, "RTBufferData");
	list->DispatchSized(Target->GetWidth(), Target->GetHeight(), 1);
	list->UAVBarrier(Target);


	//BFilterParams.Res = Screen::GetScaledRes();
	//FilterData->UpdateConstantBuffer(&BFilterParams);
	//list->SetPipelineStateDesc(RHIPipeLineStateDesc::CreateDefault(BilateralFilter));
	//list->SetUAV(Target, "sTexture");
	//list->SetConstantBufferView(FilterData, 0, "Data");
	//list->DispatchSized(Target->GetWidth(), Target->GetHeight(), 1);
	//list->UAVBarrier(Target);


	//accumulate Over N Frames
	list->SetPipelineStateDesc(RHIPipeLineStateDesc::CreateDefault(TemporalShader));
	list->SetUAV(Target, "RTXUAV");
	list->SetUAV(GetFrameBufferFromInput(2), "LastFrameData"); 
	list->SetUAV(GetFrameBufferFromInput(3), "SSP_Data");
	list->SetConstantBufferView(DenoiserData, 0, "ShaderData");
	list->DispatchSized(Target->GetWidth(), Target->GetHeight(), 1);
	list->UAVBarrier(Target);
	list->UAVBarrier(GetFrameBufferFromInput(3));
	list->UAVBarrier(GetFrameBufferFromInput(2));
	SetEndStates(list);
	Context->GetListPool()->Flush();
}

void RTFilterNode::RefreshNode()
{

}

void RTFilterNode::OnSetupNode()
{
	MergeShader = new Shader_Pair(Context, { "Raytracing\\MergeVXRT" }, { EShaderType::SHADER_COMPUTE });
	TemporalShader = new Shader_Pair(Context, { "Raytracing\\TemporalAccumilate" }, { EShaderType::SHADER_COMPUTE });
	BilateralFilter = new Shader_Pair(Context, { "Raytracing\\Denosier\\BilateralFilterCS" }, { EShaderType::SHADER_COMPUTE });
	DenoiserData = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
	DenoiserData->CreateConstantBuffer(sizeof(DenoiserParams),1);
	FilterData = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
	FilterData->CreateConstantBuffer(sizeof(FilterParams), 1);
}

void RTFilterNode::ClearAccumlate()
{}

void RTFilterNode::OnNodeSettingChange()
{
	AddResourceInput(EStorageType::Framebuffer, EResourceState::UAV, StorageFormats::DefaultFormat, "OutputBuffer");
	AddResourceInput(EStorageType::Framebuffer, EResourceState::Non_PixelShader, StorageFormats::DefaultFormat, "Voxel RT Buffer");
	AddResourceInput(EStorageType::Framebuffer, EResourceState::UAV, StorageFormats::DefaultFormat, "AccumBuffer");
	AddResourceInput(EStorageType::Framebuffer, EResourceState::UAV, StorageFormats::DefaultFormat, "SSP");
	AddOutput(EStorageType::Framebuffer, StorageFormats::ScreenReflectionData, "Screen Data");
	LinkThough(0);
}
