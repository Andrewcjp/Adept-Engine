#include "NavigationManager.h"
#include "AI/Core/AISystem.h"
#include "AI/Generation/NavMeshGenerator.h"
#include "Core/Utils/DebugDrawers.h"
#include "glm/gtx/compatibility.hpp"
#include "NavigationObstacle.h"
#include "Rendering/Core/DebugLineDrawer.h"
#include "DLTEPathfinder.h"
#include "Core/Performance/PerfManager.h"
#include "RHI/RHI.h"

#define THREAD_PATHFINDING 0
NavigationManager::NavigationManager()
{
	DPathFinder = new DLTEPathfinder();
}

NavigationManager::~NavigationManager()
{
	SafeDelete(DPathFinder);
}

void NavigationManager::RenderMesh()
{
	if (Plane == nullptr)
	{
		return;
	}
	Plane->RenderMesh(true);
}

ENavRequestStatus::Type NavigationManager::CalculatePath(glm::vec3 Startpoint, glm::vec3 EndPos, NavigationPath* outputPath)
{
	CheckNAN(Startpoint);
	Startpoint.y = 0;
	EndPos.y = 0;
	outputPath->Positions.clear();
	switch (AISystem::GetPathMode())
	{
	case EAINavigationMode::AStar:
		return CalculatePath_ASTAR(Startpoint, EndPos, outputPath);
	case EAINavigationMode::DStarLTE:
		return CalculatePath_DSTAR_LTE(Startpoint, EndPos, outputPath);
	}
	return ENavRequestStatus::Failed;
}

glm::vec3 GetPointOnBezierCurve(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, float t)
{
	glm::vec3 a = glm::lerp(p0, p1, t);
	glm::vec3 b = glm::lerp(p1, p2, t);
	glm::vec3 c = glm::lerp(p2, p3, t);
	glm::vec3 d = glm::lerp(a, b, t);
	glm::vec3 e = glm::lerp(b, c, t);
	glm::vec3 pointOnCurve = glm::lerp(d, e, t);

	return pointOnCurve;
}

void NavigationManager::SmoothPath(NavigationPath* path)
{
	if (path->Positions.size() < 4)
	{
		return;
	}
#if 0
	const int SamplingFactor = 8;
	const int Points = 2;
	for (int i = 0; i < path->Positions.size() - Points; i += Points + SamplingFactor)
	{
#if 0
		glm::vec3 point1 = GetPointOnBezierCurve(path->Positions[i], path->Positions[i + 1], path->Positions[i + 2], path->Positions[i + 3], 0.2f);
		glm::vec3 point2 = GetPointOnBezierCurve(path->Positions[i], path->Positions[i + 1], path->Positions[i + 2], path->Positions[i + 3], 0.4f);
		glm::vec3 point3 = GetPointOnBezierCurve(path->Positions[i], path->Positions[i + 1], path->Positions[i + 2], path->Positions[i + 3], 0.6f);
		glm::vec3 point4 = GetPointOnBezierCurve(path->Positions[i], path->Positions[i + 1], path->Positions[i + 2], path->Positions[i + 3], 0.8f);
		path->Positions[i] = point1;
		path->Positions[i + 1] = point2;
		path->Positions[i + 2] = point3;
		path->Positions[i + 3] = point4;
#else
		glm::vec3 dir = path->Positions[i] - path->Positions[i + 1];
		glm::vec3 controlPoint = path->Positions[i] + glm::normalize(dir) * 10;
		glm::vec3 controlPoint2 = path->Positions[i + 1] - glm::normalize(dir) * 10;
		for (int s = 0; s < SamplingFactor; s++)
		{
			const float t = (float)s / (float)SamplingFactor;

			glm::vec3 point1 = GetPointOnBezierCurve(controlPoint, path->Positions[i], path->Positions[i + 1], controlPoint2, t);
			path->Positions.insert(path->Positions.begin() + i + s / Points, point1);
		}
#endif
	}
#endif
}


ENavRequestStatus::Type NavigationManager::ValidateRequest(glm::vec3 Startpoint, glm::vec3 EndPos, NavigationPath * outpath)
{
	if (Plane == nullptr)
	{
		return ENavRequestStatus::Failed;
	}
	Tri* StartTri = Plane->FindTriangleFromWorldPos(Startpoint);
	if (StartTri == nullptr)
	{
		return ENavRequestStatus::FailedPointOffNavMesh;
	}
	Tri* EndTri = Plane->FindTriangleFromWorldPos(EndPos);
	if (EndTri == nullptr)
	{
		return ENavRequestStatus::FailedPointOffNavMesh;
	}
	if (StartTri == EndTri)
	{
		//we are within a nav triangle so we path straight to the point 
		outpath->Positions.push_back(EndPos);
		outpath->PathReady = true;
	}
	return ENavRequestStatus::Complete;
}

ENavRequestStatus::Type NavigationManager::CalculatePath_DSTAR_LTE(glm::vec3 Startpoint, glm::vec3 EndPos, NavigationPath * outputPath)
{
	const char* TimerName = "Path Compute";
	PerfManager::Get()->StartSingleActionTimer(TimerName);
	DPathFinder->Plane = Plane;
	DPathFinder->SetTarget(EndPos, Startpoint);
	DPathFinder->Execute(outputPath->Positions);

	outputPath->Positions.push_back(EndPos);
	SmoothPath(outputPath);
	if (AISystem::GetDebugMode() == EAIDebugMode::PathOnly || AISystem::GetDebugMode() == EAIDebugMode::All)
	{
		for (int i = 0; i < outputPath->Positions.size(); i++)
		{
			if (i < outputPath->Positions.size() - 1 && DebugLineDrawer::Get() != nullptr)
			{
				const float height = -10.0f;
				DebugLineDrawer::Get()->AddLine(glm::vec3(outputPath->Positions[i].x, height, outputPath->Positions[i].z),
					glm::vec3(outputPath->Positions[i + 1].x, height, outputPath->Positions[i + 1].z), glm::vec3(0, 1, 0), 1.0f);
			}
		}
	}
	PerfManager::Get()->EndSingleActionTimer(TimerName);
	PerfManager::Get()->LogSingleActionTimer(TimerName);
	PerfManager::Get()->FlushSingleActionTimer(TimerName);
	outputPath->PathReady = true;
	outputPath->PathComplete = false;
	return ENavRequestStatus::Complete;
}

ENavRequestStatus::Type NavigationManager::CalculatePath_ASTAR(glm::vec3 Startpoint, glm::vec3 EndPos, NavigationPath * outpath)
{
	return ENavRequestStatus::FailedNotSupported;
}

void NavigationManager::RegisterObstacle(NavigationObstacle * NewObstacle)
{
	Obstacles.push_back(NewObstacle);
	NewObstacle->LinkToMesh(this);
	NotifyNavMeshUpdate();
}

void NavigationManager::NotifyNavMeshUpdate()
{
	NavMeshNeedsUpdate = true;
}

std::string NavigationManager::GetErrorCodeAsString(ENavRequestStatus::Type t)
{
	switch (t)
	{
	case ENavRequestStatus::Failed:
		return "No Path";
	case ENavRequestStatus::FailedPointOffNavMesh:
		return "Point Off NavMesh";
	case ENavRequestStatus::FailedNotSupported:
		return "Not Supported";
	case ENavRequestStatus::Complete:
		return "Complete";
	}
	return "Unknown Error";
}

void NavigationManager::TickPathFinding()
{
	
#if THREAD_PATHFINDING

#else
	SCOPE_CYCLE_COUNTER("PathFind Tick");
	for (int i = 0; i < RequestsPerFrame; i++)
	{
		if (Requests.size() == 0)
		{
			return;
		}
		NavPathRequest* req = Requests.front();
		Requests.pop();
		if (req->IsRequestValid)
		{
			CalculatePath(req->StartPos, req->EndPos, req->NavPathObject);
		}
		req->NavPathObject->Request = nullptr;
		req->NavPathObject = nullptr;
		SafeDelete(req);
	}
#endif
}

ENavRequestStatus::Type NavigationManager::EnqueuePathRequest(glm::vec3 startpos, glm::vec3 endpos, NavigationPath* outpath)
{
	ENavRequestStatus::Type result = ValidateRequest(startpos, endpos, outpath);
	if (result != ENavRequestStatus::Complete)
	{
		return result;
	}
	NavPathRequest* req = new NavPathRequest();
	if (outpath->Request != nullptr)
	{
		return ENavRequestStatus::Failed;
	}
	req->NavPathObject = outpath;
	req->StartPos = startpos;
	req->EndPos = endpos;
	req->IsRequestValid = true;
	outpath->Request = req;
	Requests.emplace(req);
	return ENavRequestStatus::Complete;
}

void NavigationPath::EndPath()
{
	PathComplete = false;
	Positions.clear();
}
