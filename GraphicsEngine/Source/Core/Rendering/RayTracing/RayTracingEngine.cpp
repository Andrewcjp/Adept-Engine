#include "RayTracingEngine.h"
#include "HighLevelAccelerationStructure.h"
#include "LowLevelAccelerationStructure.h"
#include "RHI/DeviceContext.h"
#include "Core/Assets/ShaderComplier.h"
#include "RHIStateObject.h"
#include "Shader_RTBase.h"
#include "Core/BaseWindow.h"
#include "RayTracingCommandList.h"
#include "ShaderBindingTable.h"


RayTracingEngine::RayTracingEngine()
{
	AsyncbuildList = RHI::CreateCommandList(ECommandListType::Compute);
	RTList = CreateRTList(RHI::GetDefaultDevice());
	DefaultTable = new ShaderBindingTable();
	StateObject = RHI::GetRHIClass()->CreateStateObject(RHI::GetDefaultDevice());
	StateObject->ShaderTable = DefaultTable;
	StateObject->TempCam = BaseWindow::GetCurrentCamera();
	StateObject->Build();
}

RayTracingEngine::~RayTracingEngine()
{}

RayTracingEngine * RayTracingEngine::Get()
{
	return RHI::instance->RTE;
}

void RayTracingEngine::EnqueueForBuild(LowLevelAccelerationStructure * Struct)
{
	LASToBuild.push_back(Struct);
	CurrnetHL->AddEntity(Struct);
}

void RayTracingEngine::EnqueueForBuild(HighLevelAccelerationStructure * Struct)
{
	HASToBuild.push_back(Struct);
}

void RayTracingEngine::BuildForFrame(RHICommandList* List)
{
	if (LASToBuild.size() == 0)
	{
		//CurrnetHL->Build(List);
		CurrnetHL->Update(List);
		return;
	}
	for (int i = 0; i < LASToBuild.size(); i++)
	{
		LASToBuild[i]->Build(List);
	}
	CurrnetHL->Build(List);
	Build = true;
	LASToBuild.clear();
}

void RayTracingEngine::DispatchRaysForMainScenePass(FrameBuffer* Target)
{
	if (RHI::GetFrameCount() == 1)
	{
		StateObject->RebuildShaderTable();
	}
	StateObject->TempCam = BaseWindow::GetCurrentCamera();
	RTList->GetRHIList()->GetDevice()->InsertGPUWait(DeviceContextQueue::Compute, DeviceContextQueue::Graphics);

	RTList->ResetList();
	RTList->GetRHIList()->StartTimer(EGPUTIMERS::RT_Trace);
	RTList->SetStateObject(StateObject);
	Target->MakeReadyForComputeUse(RTList->GetRHIList());
	RTList->SetHighLevelAccelerationStructure(CurrnetHL);
	RTList->TraceRays(RHIRayDispatchDesc(Target));
	RTList->GetRHIList()->EndTimer(EGPUTIMERS::RT_Trace);
	RTList->Execute();

	RTList->GetRHIList()->GetDevice()->InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::Compute);
}

void RayTracingEngine::SetShaderTable(ShaderBindingTable* SBT)
{

}

void RayTracingEngine::OnFirstFrame()
{
	CurrnetHL = RHI::GetRHIClass()->CreateHighLevelAccelerationStructure(RHI::GetDefaultDevice());
	for (int i = 0; i < LASToBuild.size(); i++)
	{
		CurrnetHL->AddEntity(LASToBuild[i]);
	}
}

void RayTracingEngine::BuildStructures()
{
	if (!Build)
	{
		CurrnetHL->InitialBuild();
	}
	AsyncbuildList->ResetList();
	BuildForFrame(AsyncbuildList);
	AsyncbuildList->Execute();
	RHI::GetDefaultDevice()->InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::Compute);
	//RHI::WaitForGPU();
}

RayTracingCommandList * RayTracingEngine::CreateRTList(DeviceContext * Device)
{
	return new RayTracingCommandList(Device, ERayTracingSupportType::DriverBased);
}

void RayTracingEngine::TraceRaysForReflections(FrameBuffer * Target, FrameBuffer* NormalSrcBuffer)
{

}