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
}

RayTracingEngine::~RayTracingEngine()
{
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
}

RayTracingCommandList * RayTracingEngine::CreateRTList(DeviceContext * Device)
{
	return new RayTracingCommandList(Device);
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

HighLevelAccelerationStructure * RayTracingEngine::GetHighLevelStructure()
{
	return CurrnetHL;
}
