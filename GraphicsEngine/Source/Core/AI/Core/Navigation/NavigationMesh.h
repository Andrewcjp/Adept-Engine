#pragma once
#include "Core/Utils/MathUtils.h"
struct NavigationPath
{
	bool PathReady = false;
	std::vector<glm::vec3> Positions;
};
namespace ENavRequestStatus
{
	enum Type
	{
		Failed,
		FailedPointOffNavMesh,
		FailedNotSupported,
		Complete,
	};
}
class NavigationObstacle;
struct NavPlane;
class DLTEPathfinder;
class NavigationMesh
{
public:

	NavigationMesh();
	~NavigationMesh();
	void RenderMesh();
	ENavRequestStatus::Type CalculatePath(glm::vec3 Startpoint, glm::vec3 EndPos, NavigationPath** outputPath);
	void RegisterObstacle(NavigationObstacle* NewObstacle);
	void NotifyNavMeshUpdate();
	static std::string GetErrorCodeAsString(ENavRequestStatus::Type t);
	NavPlane* Plane = nullptr;
private:
	DLTEPathfinder* DPathFinder = nullptr;
	ENavRequestStatus::Type CalculatePath_ASTAR(glm::vec3 Startpoint, glm::vec3 EndPos, NavigationPath ** outpath);
	ENavRequestStatus::Type CalculatePath_DSTAR_LTE(glm::vec3 Startpoint, glm::vec3 EndPos, NavigationPath ** outpath);
	ENavRequestStatus::Type CalculatePath_DSTAR_BoardPhase(glm::vec3 Startpoint, glm::vec3 EndPos, NavigationPath ** outpath);
	void SmoothPath(NavigationPath * path);	
	bool NavMeshNeedsUpdate = false;
	std::vector<NavigationObstacle*> Obstacles;
};

