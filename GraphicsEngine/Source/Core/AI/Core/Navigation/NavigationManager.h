#pragma once
#include "Core/Utils/MathUtils.h"
struct NavPathRequest;
struct NavigationPath
{
	bool PathReady = false;
	std::vector<glm::vec3> Positions;
	NavPathRequest* Request = nullptr;
	bool PathComplete = false;
	void EndPath();
};
struct NavPathRequest
{
	glm::vec3 StartPos = glm::vec3(0);
	glm::vec3 EndPos = glm::vec3(0);
	NavigationPath* NavPathObject = nullptr;
	bool IsRequestValid = true;
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
class NavigationManager
{
public:

	NavigationManager();
	~NavigationManager();
	void RenderMesh();

	void RegisterObstacle(NavigationObstacle* NewObstacle);
	void NotifyNavMeshUpdate();
	static std::string GetErrorCodeAsString(ENavRequestStatus::Type t);
	NavPlane* Plane = nullptr;
	void TickPathFinding();
	ENavRequestStatus::Type EnqueuePathRequest(glm::vec3 startpos, glm::vec3 endpos, NavigationPath* outpath);
private:
	ENavRequestStatus::Type CalculatePath(glm::vec3 Startpoint, glm::vec3 EndPos, NavigationPath* outputPath);
	DLTEPathfinder* DPathFinder = nullptr;
	ENavRequestStatus::Type CalculatePath_ASTAR(glm::vec3 Startpoint, glm::vec3 EndPos, NavigationPath * outpath);
	ENavRequestStatus::Type CalculatePath_DSTAR_LTE(glm::vec3 Startpoint, glm::vec3 EndPos, NavigationPath * outpath);

	void RenderPath(NavigationPath * outputPath, glm::vec3 Colour = glm::vec3(0, 1, 0));

	ENavRequestStatus::Type ValidateRequest(glm::vec3 Startpoint, glm::vec3 EndPos, NavigationPath * outpath);

	void SmoothPath(NavigationPath * path);
	bool NavMeshNeedsUpdate = false;
	std::vector<NavigationObstacle*> Obstacles;
	std::queue<NavPathRequest*> Requests;
	int RequestsPerFrame = 1;
};

