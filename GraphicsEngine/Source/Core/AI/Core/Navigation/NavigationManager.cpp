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
	Plane->RenderMesh(false);
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
	const float tollecance = 0.1f;
	float change = tollecance;
	const float weight_data = 0.25f;
	const float weight_smooth = 0.1f;
	const int IterationLimit = 30;
	int StartValue = 1;
	const int SampleSize = 2;//Smooth path without loosing too much definition
	for (int sec = 0; sec < path->Positions.size() / SampleSize; sec++)
	{
		if (sec != 0)
		{
			StartValue = sec * SampleSize;
		}
		int CurrnetItor = 0;
		while (change >= tollecance && CurrnetItor < IterationLimit)
		{
			CurrnetItor++;
			change = 0.0f;
			for (int i = StartValue; i < glm::min((int)path->Positions.size() - 2, StartValue + SampleSize); i++)
			{

				for (int j = 0; j < 3; j++)
				{
					float x_i = path->Positions[i][j];
					float y_i = path->Positions[i][j];
					float Y_prev = path->Positions[i - 1][j];
					float Y_next = path->Positions[i + 1][j];
					float y_i_saved = y_i;

					y_i += weight_data * (x_i - y_i) + weight_smooth * (Y_next + Y_prev - (2 * y_i));
					path->Positions[i][j] = y_i;
					change += abs(y_i - y_i_saved);
				}
			}
		}
		change = tollecance;
	}
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
		DLTENode* node;
		if (Plane->ResolvePositionToNode(Startpoint, &node))
		{
			if (node == nullptr)
			{
				return ENavRequestStatus::FailedStartPointOffNavMesh;
			}
			StartTri = node->OwnerTri;
		}
		else
		{
			return ENavRequestStatus::FailedStartPointOffNavMesh;
		}
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
	if (!DPathFinder->Execute(outputPath->Positions))
	{
		Log::LogMessage("Failed to find path");
		return ENavRequestStatus::Failed;
	}

	outputPath->Positions.push_back(EndPos);
	RenderPath(outputPath, glm::vec3(1, 0, 0));
	SmoothPath(outputPath);
	RenderPath(outputPath);

	PerfManager::Get()->EndSingleActionTimer(TimerName);
	PerfManager::Get()->LogSingleActionTimer(TimerName);
	PerfManager::Get()->FlushSingleActionTimer(TimerName);
	outputPath->PathReady = true;
	outputPath->PathComplete = false;
	return ENavRequestStatus::Complete;
}

void NavigationManager::RenderPath(NavigationPath * outputPath, glm::vec3 Colour /*= glm::vec3(1)*/)
{
	if (AISystem::GetDebugMode() == EAIDebugMode::PathOnly || AISystem::GetDebugMode() == EAIDebugMode::All)
	{
		for (int i = 0; i < outputPath->Positions.size(); i++)
		{
			if (i < outputPath->Positions.size() - 1 && DebugLineDrawer::Get() != nullptr)
			{
				DebugLineDrawer::Get()->AddLine(glm::vec3(outputPath->Positions[i].x, outputPath->Positions[i].y, outputPath->Positions[i].z),
					glm::vec3(outputPath->Positions[i + 1].x, outputPath->Positions[i + 1].y, outputPath->Positions[i + 1].z), Colour, 1.0f);
			}
		}
	}
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
	case ENavRequestStatus::FailedStartPointOffNavMesh:
		return "Start Point Off NavMesh";
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
