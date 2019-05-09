#include "CullingManager.h"
#include "Core\Assets\Scene.h"
#include "Core\Utils\DebugDrawers.h"

static ConsoleVariable Freeze("c.Freeze", 0);
static ConsoleVariable ShowAABB("c.ShowBounds", 0);

CullingManager::CullingManager()
{}


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
			bool culled = CurrentObj->IsCulled();
			CurrentObj->GetBounds()->DebugRender(culled ? glm::vec3(1, 0, 0) : glm::vec3(1, 1, 1));
		}
	}
	//Frustum.DebugRender();
}

void CullingManager::UpdateMainPassCulling(Camera * maincam, Scene * target)
{
	DebugRender(maincam, target);
	if (Freeze.GetBoolValue())
	{
		return;
	}
	Frustum.SetupFromCamera(maincam);
	for (int i = 0; i < (*target->GetMeshObjects()).size(); i++)
	{
		GameObject* CurrentObj = (*target->GetMeshObjects())[i];
		if (CurrentObj->GetMesh() != nullptr)
		{
			CurrentObj->SetCulledState(!Frustum.TestObject(CurrentObj->GetBounds()));
		}
	}
}

void CullingManager::UpdateCullingForShadowLight(Light* light)
{
	//run distance culling
}


