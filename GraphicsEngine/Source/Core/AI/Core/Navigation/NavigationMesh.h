#pragma once
#include "Core/Utils/MathUtils.h"
struct NavigationPath
{
	bool PathReady = false;
	std::vector<glm::vec3> Positions;
};
struct NavPathRequest 
{
	glm::vec3 StartPos = glm::vec3(0);
	glm::vec3 EndPos = glm::vec3(0);
	NavigationPath** NavPathObject = nullptr;
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
class NavPlane;
class DLTEPathfinder;
class NavigationMesh
{
public:

	NavigationMesh();
	~NavigationMesh();
	void RenderMesh();

	void RegisterObstacle(NavigationObstacle* NewObstacle);
	void NotifyNavMeshUpdate();
	static std::string GetErrorCodeAsString(ENavRequestStatus::Type t);
	NavPlane* Plane = nullptr;
	void TickPathFinding();
	ENavRequestStatus::Type EnqueuePathRequest(glm::vec3 startpos, glm::vec3 endpos, NavigationPath** outpath);
private:
	ENavRequestStatus::Type CalculatePath(glm::vec3 Startpoint, glm::vec3 EndPos, NavigationPath** outputPath);
	DLTEPathfinder* DPathFinder = nullptr;
	ENavRequestStatus::Type CalculatePath_ASTAR(glm::vec3 Startpoint, glm::vec3 EndPos, NavigationPath ** outpath);	
	ENavRequestStatus::Type CalculatePath_DSTAR_LTE(glm::vec3 Startpoint, glm::vec3 EndPos, NavigationPath ** outpath);
	ENavRequestStatus::Type CalculatePath_DSTAR_BoardPhase(glm::vec3 Startpoint, glm::vec3 EndPos, NavigationPath ** outpath);
	ENavRequestStatus::Type ValidateRequest(glm::vec3 Startpoint, glm::vec3 EndPos, NavigationPath ** outpath);

	void SmoothPath(NavigationPath * path);	
	bool NavMeshNeedsUpdate = false;
	std::vector<NavigationObstacle*> Obstacles;
	std::queue<NavPathRequest*> Requests;
	int RequestsPerFrame = 1;
};

