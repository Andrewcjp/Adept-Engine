#include "CullingManager.h"
#include "Core\Assets\Scene.h"
#include "Core\Utils\DebugDrawers.h"
#include "Core\Performance\PerfManager.h"
#include "Core\Platform\Threading.h"
#define USE_TASKGRAPH 0
static ConsoleVariable Freeze("c.Freeze", false);
static ConsoleVariable ShowAABB("c.ShowBounds", false);
const std::string MPObjects = "Main Pass Objects";
const std::string SWObjects = "Shadow Pass Objects";

CullingManager::CullingManager()
{
	PerfManager::Get()->AddTimer(MPObjects.c_str(), "Render");
	PerfManager::Get()->AddTimer(SWObjects.c_str(), "Render");
}


CullingManager::~CullingManager()
{}

void CullingManager::DebugRender(Camera * maincam, Scene * target)
{
	if (!ShowAABB.GetBoolValue())
	{
		return;
	}
	for (int i = 0; i < target->GetMeshObjects().size(); i++)
	{
		GameObject* CurrentObj = target->GetMeshObjects()[i];
		if (CurrentObj->GetMesh() != nullptr)
		{
			bool culled = CurrentObj->IsCulled(ECullingPass::MainPass);
			CurrentObj->GetBounds()->DebugRender(culled ? glm::vec3(1, 0, 0) : glm::vec3(1, 1, 1));
		}
	}
	//Frustum.DebugRender();
}

void CullingManager::UpdateMainPassFrustumCulling(Camera * maincam, Scene * target)
{
	SCOPE_CYCLE_COUNTER_GROUP("MainPassFrustumCulling", "Culling");
	DebugRender(maincam, target);
	if (Freeze.GetBoolValue())
	{
		return;
	}
	Frustum.SetupFromCamera(maincam);
	int cullcount = 0;
#if USE_TASKGRAPH
	const int ThreadCount = 6;
	int BatchSize = target->GetMeshObjects().size() /ThreadCount;
	std::function <void(int)> ProcessCullingTask = [&](int threadIndex)
	{
		const int StartingIndex = threadIndex * BatchSize;
		int ThisBatchcount = glm::min(StartingIndex + BatchSize, (int)target->GetMeshObjects().size());
		for (int i = StartingIndex; i < ThisBatchcount; i++)
		{
			GameObject* CurrentObj = target->GetMeshObjects()[i];
			ProcessObject(CurrentObj, maincam, cullcount);
		}
	};
	Engine::GetTaskGraph()->RunTaskOnGraph(ProcessCullingTask, ThreadCount);

#else
	for (int i = 0; i < target->GetMeshObjects().size(); i++)
	{
		GameObject* CurrentObj = target->GetMeshObjects()[i];	
		if (!CurrentObj->GetIsActive())
		{
			continue;
		}
		ProcessObject(CurrentObj, maincam, cullcount);
	}
#endif
	PerfManager::AddToCountTimer(MPObjects, cullcount);
}

void CullingManager::ProcessObject(GameObject* CurrentObj, Camera * maincam, int& cullcount)
{
	if (CurrentObj->DisableCulling)
	{
		CurrentObj->SetCulledState(ECullingPass::MainPass, false);
		return;
	}
	if (CurrentObj->GetMesh() != nullptr)
	{
		if (!CurrentObj->IsOnLayer(maincam->RenderMask))
		{
			CurrentObj->SetCulledState(ECullingPass::MainPass, true);
			return;
		}
		const bool culled = !Frustum.TestObject(CurrentObj->GetBounds());
		if (!culled)
		{
			cullcount++;
		}
		CurrentObj->SetCulledState(ECullingPass::MainPass, culled);
	}	
}

void CullingManager::UpdateCullingForShadowLight(Light* light, Scene* target)
{
	SCOPE_CYCLE_COUNTER_GROUP("Shadow Culling", "Culling");
	//run distance culling
	int Count = 0;
	for (int i = 0; i < target->GetMeshObjects().size(); i++)
	{
		GameObject* CurrentObj = target->GetMeshObjects()[i];
		if (CurrentObj->GetMesh() != nullptr)
		{
			const float distance = glm::distance2(CurrentObj->GetPosition(), light->GetPosition());
			const bool culled = distance > light->GetRange()*light->GetRange();
			if (!culled)
			{
				Count++;
			}
			CurrentObj->SetCulledState(ECullingPass::ShadowPass, culled);
		}
	}
	PerfManager::AddToCountTimer(SWObjects, Count);
}

CullingFrustum * CullingManager::GetFustrum()
{
	return &Frustum;
}


