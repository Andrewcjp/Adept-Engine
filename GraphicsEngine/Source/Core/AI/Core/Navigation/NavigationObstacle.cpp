
#include "NavigationObstacle.h"
#include "NavigationMesh.h"

NavigationObstacle::NavigationObstacle()
{
}


NavigationObstacle::~NavigationObstacle()
{
}

void NavigationObstacle::LinkToMesh(NavigationMesh * parentmesh)
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
