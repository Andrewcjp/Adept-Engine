#include "NavigationMesh.h"
#include "AI/Core/AISystem.h"
#include "AI/Generation/NavMeshGenerator.h"
#include "Core/Utils/DebugDrawers.h"
#include "glm/gtx/compatibility.hpp"
#include "NavigationObstacle.h"
#include "Rendering/Core/DebugLineDrawer.h"
#include "DLTEPathfinder.h"


NavigationMesh::NavigationMesh()
{
	DPathFinder = new DLTEPathfinder();
}

NavigationMesh::~NavigationMesh()
{
	SafeDelete(DPathFinder);
}

void NavigationMesh::RenderMesh()
{
	if (Plane == nullptr)
	{
		return;
	}
	for (int i = 0; i < Plane->Triangles.size(); i++)
	{
		const int sides = 3;
		for (int x = 0; x < sides; x++)
		{
			const int next = (x + 1) % sides;
			DebugDrawers::DrawDebugLine(Plane->Triangles[i].points[x], Plane->Triangles[i].points[next], glm::vec3(1), false, 0.0f);
		}
	}
}

ENavRequestStatus::Type NavigationMesh::CalculatePath(glm::vec3 Startpoint, glm::vec3 EndPos, NavigationPath** outpath)
{
	Startpoint.y = 0;
	EndPos.y = 0;
	switch (AISystem::GetPathMode())
	{
	case EAINavigationMode::AStar:
		return CalculatePath_ASTAR(Startpoint, EndPos, outpath);
	case EAINavigationMode::DStarLTE:
		return CalculatePath_DSTAR_LTE(Startpoint, EndPos, outpath);
	case EAINavigationMode::DStarBoardPhase:
		return CalculatePath_DSTAR_BoardPhase(Startpoint, EndPos, outpath);
	}
	return ENavRequestStatus::Failed;
}

ENavRequestStatus::Type NavigationMesh::CalculatePath_DSTAR_BoardPhase(glm::vec3 Startpoint, glm::vec3 EndPos, NavigationPath** outpath)
{
	return ENavRequestStatus::FailedNotSupported;
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

void NavigationMesh::SmoothPath(NavigationPath* path)
{
	if (path->Positions.size() < 4)
	{
		return;
	}
	return;
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
}

ENavRequestStatus::Type NavigationMesh::CalculatePath_DSTAR_LTE(glm::vec3 Startpoint, glm::vec3 EndPos, NavigationPath** outpath)
{
	if (Plane == nullptr)
	{
		return ENavRequestStatus::Failed;
	}
	NavigationPath* outputPath = new NavigationPath();
	*outpath = outputPath;
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
		outputPath->Positions.push_back(EndPos);
		return ENavRequestStatus::Complete;
	}
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

	return ENavRequestStatus::Complete;
}

ENavRequestStatus::Type NavigationMesh::CalculatePath_ASTAR(glm::vec3 Startpoint, glm::vec3 EndPos, NavigationPath** outpath)
{
	return ENavRequestStatus::FailedNotSupported;
}

void NavigationMesh::RegisterObstacle(NavigationObstacle * NewObstacle)
{
	Obstacles.push_back(NewObstacle);
	NewObstacle->LinkToMesh(this);
	NotifyNavMeshUpdate();
}

void NavigationMesh::NotifyNavMeshUpdate()
{
	NavMeshNeedsUpdate = true;
}

std::string NavigationMesh::GetErrorCodeAsString(ENavRequestStatus::Type t)
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


