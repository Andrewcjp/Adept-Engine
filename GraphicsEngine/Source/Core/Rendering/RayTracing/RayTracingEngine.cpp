#include "RayTracingEngine.h"
#include "Core/Assets/ShaderComplier.h"
#include "Core/BaseWindow.h"
#include "HighLevelAccelerationStructure.h"
#include "LowLevelAccelerationStructure.h"
#include "RayTracingCommandList.h"
#include "Rendering/Core/SceneRenderer.h"
#include "Rendering/Renderers/RenderEngine.h"
#include "RHI/DeviceContext.h"
#include "RHIStateObject.h"
#include "Shader_RTBase.h"
#include "ShaderBindingTable.h"
#include "Core/Assets/Scene.h"
#include "../Core/LightCulling/LightCullingEngine.h"
#include "../RenderNodes/StoreNodes/ShadowAtlasStorageNode.h"

RayTracingEngine::RayTracingEngine()
{
	AsyncbuildList = RHI::CreateCommandList(ECommandListType::Compute);
	RTList = CreateRTList(RHI::GetDefaultDevice());
	DefaultTable = new ShaderBindingTable();
	if (RHI::GetRenderSettings()->GetRTSettings().UseForMainPass)
	{
		DefaultTable->InitDefault();
	}
	else if (RHI::GetRenderSettings()->GetRTSettings().UseForReflections)
	{
		DefaultTable->InitReflections();
	}
	StateObject = RHI::GetRHIClass()->CreateStateObject(RHI::GetDefaultDevice());
	StateObject->ShaderTable = DefaultTable;
	//StateObject->TempCam = SceneRenderer::Get()->GetCurrentCamera();
	StateObject->Build();
	AddHitTable(DefaultTable);
}

RayTracingEngine::~RayTracingEngine()
{
	SafeRelease(RTList);
	SafeDelete(DefaultTable);
	SafeRelease(StateObject);
	SafeRelease(CurrnetHL);
}

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
		//if (RHI::GetFrameCount() == 10)
		{
			//CurrnetHL->Build(List);
		}
		//CurrnetHL->Update(List);
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
	if (RHI::GetFrameCount() == 0)
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
	//RHI::WaitForGPU();
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
	return new RayTracingCommandList(Device);
}

void RayTracingEngine::TraceRaysForReflections(FrameBuffer * Target, FrameBuffer* NormalSrcBuffer, ShadowAtlasStorageNode* shadow)
{
	if (RHI::GetFrameCount() == 0)
	{
		StateObject->RebuildShaderTable();
	}
	ensure(NormalSrcBuffer);
	StateObject->TempCam = BaseWindow::GetCurrentCamera();
	RTList->GetRHIList()->GetDevice()->InsertGPUWait(DeviceContextQueue::Compute, DeviceContextQueue::Graphics);

	RTList->ResetList();
	RTList->GetRHIList()->StartTimer(EGPUTIMERS::RT_Trace);
	RTList->SetStateObject(StateObject);
	RTList->GetRHIList()->SetFrameBufferTexture(NormalSrcBuffer, 3, 1);
	RTList->GetRHIList()->SetFrameBufferTexture(NormalSrcBuffer, 4, 0);
	SceneRenderer::Get()->BindLightsBuffer(RTList->GetRHIList(), 5);
	SceneRenderer::Get()->GetLightCullingEngine()->GetLightDataBuffer()->BindBufferReadOnly(RTList->GetRHIList(), 6);
	//Target->MakeReadyForComputeUse(RTList->GetRHIList());
	shadow->BindPointArray(RTList->GetRHIList(), 7);


	RTList->SetHighLevelAccelerationStructure(CurrnetHL);
	RTList->TraceRays(RHIRayDispatchDesc(Target));
	RTList->GetRHIList()->EndTimer(EGPUTIMERS::RT_Trace);
	RTList->Execute();

	RTList->GetRHIList()->GetDevice()->InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::Compute);
}

void RayTracingEngine::UpdateFromScene(Scene * S)
{
	for (ShaderBindingTable* T : Tables)
	{
		T->RebuildHittableFromScene(S);
	}
}

void RayTracingEngine::AddHitTable(ShaderBindingTable * Table)
{
	Tables.push_back(Table);
}
