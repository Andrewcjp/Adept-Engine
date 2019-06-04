#include "RayTracingEngine.h"
#include "HighLevelAccelerationStructure.h"
#include "LowLevelAccelerationStructure.h"
#include "RHI/DeviceContext.h"
#include "Core/Assets/ShaderComplier.h"
#include "RHIStateObject.h"
#include "Shader_RTBase.h"
#include "Core/BaseWindow.h"


RayTracingEngine::RayTracingEngine()
{
	AsyncbuildList = RHI::CreateCommandList(ECommandListType::Compute);
	RayList = RHI::CreateCommandList();
	StateObject = RHI::GetRHIClass()->CreateStateObject(RHI::GetDefaultDevice());
	StateObject->Target = ShaderComplier::GetShader<Shader_RTBase>();
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
	if (Build)
	{
		//CurrnetHL->Build(List);
		return;
	}
	for (int i = 0; i < LASToBuild.size(); i++)
	{
		LASToBuild[i]->Build(List);
	}
	CurrnetHL->Build(List);
	Build = true;
}

void RayTracingEngine::DispatchRays(FrameBuffer* Target)
{
	StateObject->TempCam = BaseWindow::GetCurrentCamera();
	RayList->ResetList();
	RayList->StartTimer(EGPUTIMERS::RT_Trace);
	RayList->SetStateObject(StateObject);
	RayList->BeginRenderPass(RHIRenderPassDesc(Target));
	Target->MakeReadyForComputeUse(RayList);
	RayList->SetHighLevelAccelerationStructure(CurrnetHL);
	RayList->TraceRays(RHIRayDispatchDesc(Target));
	RayList->EndRenderPass();
	RayList->EndTimer(EGPUTIMERS::RT_Trace);
	RayList->Execute();
}

void RayTracingEngine::SetShaderTable()
{}

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
