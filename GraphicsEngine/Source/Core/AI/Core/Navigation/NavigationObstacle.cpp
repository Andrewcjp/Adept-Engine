
#include "NavigationObstacle.h"
#include "NavigationManager.h"

NavigationObstacle::NavigationObstacle()
{
}


NavigationObstacle::~NavigationObstacle()
{
}

void NavigationObstacle::LinkToMesh(NavigationManager * parentmesh)
{
	ParentMesh = parentmesh;
}

void NavigationObstacle::InitComponent()
{

}

void NavigationObstacle::OnTransformUpdate()
{
	if (ParentMesh != nullptr)
	{
		ParentMesh->NotifyNavMeshUpdate();
	}
}
