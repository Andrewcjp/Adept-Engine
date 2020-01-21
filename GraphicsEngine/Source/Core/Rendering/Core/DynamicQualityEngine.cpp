#include "DynamicQualityEngine.h"
#include "RHI/RHICommandList.h"
static ConsoleVariable RT_Threshold("DQ.RT", 0.8f, ECVarType::ConsoleAndLaunch, true);
static ConsoleVariable rt_BlendSize("DQ.Blend", 0.1f, ECVarType::ConsoleAndLaunch, true);
static ConsoleVariable RT_RayCount("DQ.RT.Count", 5, ECVarType::ConsoleAndLaunch, true);
DynamicQualityEngine::DynamicQualityEngine()
{
	RTBuffer = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
	RTBuffer->CreateConstantBuffer(sizeof(RTBufferData), 1);
	RTBuffer->UpdateConstantBuffer(&RTQualityData);
	//RT_Threshold.SetValueF(1.0f);
}

DynamicQualityEngine::~DynamicQualityEngine()
{}

void DynamicQualityEngine::Update()
{
	UpdateVXRTCrossover();
}

void DynamicQualityEngine::UpdateVXRTCrossover()
{
	RTQualityData.Max_VXRayCount = RT_RayCount.GetIntValue();
	RTQualityData.Max_RTRayCount = RT_RayCount.GetIntValue();
	RTQualityData.RT_RoughnessThreshold = RT_Threshold.GetFloatValue();
	float BlendSize = rt_BlendSize.GetFloatValue();
	RTQualityData.VX_MaxRoughness = RTQualityData.RT_RoughnessThreshold + BlendSize / 2;
	RTQualityData.VX_RT_BlendStart = RTQualityData.RT_RoughnessThreshold - BlendSize / 2;
	RTQualityData.VX_RT_BlendEnd = RTQualityData.VX_MaxRoughness;
	RTQualityData.FrameCount = RHI::GetFrameCount();
	RTBuffer->UpdateConstantBuffer(&RTQualityData);
}

void DynamicQualityEngine::BindRTBuffer(RHICommandList * list, std::string Name)
{
	list->SetConstantBufferView(RTBuffer, 0, Name);
}
void DynamicQualityEngine::BindRTBuffer(RHICommandList * list, int slot)
{
	list->SetConstantBufferView(RTBuffer, 0, slot);
}
DynamicQualityEngine * DynamicQualityEngine::Get()
{
	return RHI::Get()->QualityEngine;
}
