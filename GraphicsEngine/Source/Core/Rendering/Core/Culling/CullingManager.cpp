#include "CullingManager.h"
#include "Core\Assets\Scene.h"
#include "Core\Utils\DebugDrawers.h"
#include "Core\Performance\PerfManager.h"

static ConsoleVariable Freeze("c.Freeze", 0);
static ConsoleVariable ShowAABB("c.ShowBounds", 0);
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
	for (int i = 0; i < (*target->GetMeshObjects()).size(); i++)
	{
		GameObject* CurrentObj = (*target->GetMeshObjects())[i];
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
	for (int i = 0; i < (*target->GetMeshObjects()).size(); i++)
	{
		GameObject* CurrentObj = (*target->GetMeshObjects())[i];
		if (CurrentObj->GetMesh() != nullptr)
		{
			const bool culled = !Frustum.TestObject(CurrentObj->GetBounds());
			if (!culled)
			{
				cullcount++;
			}
			CurrentObj->SetCulledState(ECullingPass::MainPass, culled);
		}
	}
	PerfManager::AddToCountTimer(MPObjects, cullcount);
}

void CullingManager::UpdateCullingForShadowLight(Light* light, Scene* target)
{
	SCOPE_CYCLE_COUNTER_GROUP("Shadow Culling", "Culling");
	//run distance culling
	for (int i = 0; i < (*target->GetMeshObjects()).size(); i++)
	{
		GameObject* CurrentObj = (*target->GetMeshObjects())[i];
		if (CurrentObj->GetMesh() != nullptr)
		{
			const float distance = glm::distance2(CurrentObj->GetPosition(), light->GetPosition());
			const bool culled = distance > light->GetRange()*light->GetRange();
			if (!culled)
			{
				PerfManager::AddToCountTimer(SWObjects, 1);
			}
			CurrentObj->SetCulledState(ECullingPass::ShadowPass, culled);
		}
	}

}


