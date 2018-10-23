#include "Stdafx.h"
#include "NavigationMesh.h"
#include "Core/Assets/MeshLoader.h"
#include "Rendering/Core/DebugLineDrawer.h"
#include "Core/Assets/AssetManager.h"
#include "AI/Core/AISystem.h"
#include "AI/Core/NavigationObstacle.h"
NavigationMesh::NavigationMesh()
{}

NavigationMesh::~NavigationMesh()
{}

void NavigationMesh::GenTestMesh()
{
	std::vector<OGLVertex> vertices;
	std::vector<int> indices;
	MeshLoader::FMeshLoadingSettings t;
	t.GenerateIndexed = false;
	t.Scale = glm::vec3(10);
	MeshLoader::LoadMeshFromFile_Direct(AssetManager::GetContentPath() + "models\\NavPlaneTest_L.obj", t, vertices, indices);

	for (int i = 0; i < vertices.size(); i += 3)
	{
		NavTriangle* a = new NavTriangle();
		a->Positons[0] = vertices[indices[i]].m_position;
		a->Positons[1] = vertices[indices[i + 1]].m_position;
		a->Positons[2] = vertices[indices[i + 2]].m_position;

		for (int x = 0; x < 3; x++)
		{
			a->avgcentre += a->Positons[x];
		}
		a->avgcentre /= 3;
		Triangles.push_back(a);
	}
	PopulateNearLists();
	//NavigationPath* data;1
	//CalculatePath(glm::vec3(20, 0, -10), glm::vec3(-15, 0, 20), &data);
}

void NavigationMesh::DrawNavMeshLines(DebugLineDrawer* drawer)
{
	glm::vec3 offset = glm::vec3(0, 0.2f, 0);
	for (int i = 0; i < Triangles.size(); i++)
	{
		for (int x = 0; x < 3; x++)
		{
			const int next = (x + 1) % 3;
			drawer->AddLine(offset + Triangles[i]->Positons[x], offset + Triangles[i]->Positons[next], glm::vec3(1, 1, 1));
		}
		//for (int n = 0; n < Triangles[i]->NearTriangles.size(); n++)
		{
			//drawer->AddLine(offset + Triangles[i]->avgcentre, offset + Triangles[i]->avgcentre+glm::vec3(0,10,0), glm::vec3(1, 0, 0));
		}
	}
}

struct Edge
{
	Edge(glm::vec3 f, glm::vec3 s)
	{
		first = f;
		second = s;
	}
	glm::vec3 first;
	glm::vec3 second;
	NavTriangle* tri;
	bool operator==(Edge lhs)
	{
		return (lhs.first == first) && (lhs.second == second) || (lhs.second == first) && (lhs.first == second);
	}
};

int findInV(std::vector<Edge>& v, Edge value)
{
	for (int i = 0; i < v.size(); i++)
	{
		if (value == v[i])
		{
			return i;
		}
	}
	return -1;
}

void NavigationMesh::PopulateNearLists()
{
	std::vector<Edge> SharedEdges;
	for (int i = 0; i < Triangles.size(); i++)
	{
		for (int x = 0; x < 3; x++)
		{
			const int next = (x + 1) % 3;
			Edge e = Edge(Triangles[i]->Positons[x], Triangles[i]->Positons[next]);
			e.tri = Triangles[i];
			int index = findInV(SharedEdges, e);
			if (index != -1)
			{
				SharedEdges[index].tri->NearTriangles.push_back(Triangles[i]);
				Triangles[i]->NearTriangles.push_back(SharedEdges[index].tri);
			}
			else
			{
				SharedEdges.push_back(e);
			}
		}
	}
}

NavTriangle* NavigationMesh::FindTriangleFromWorldPos(glm::vec3 worldpos)
{
	for (int i = 0; i < Triangles.size(); i++)
	{
		if (Triangles[i]->IsPointInsideTri(worldpos))
		{
			return Triangles[i];
		}
	}
	return nullptr;
}

void CalulateCost(NavPoint* point, glm::vec3 endpoint, glm::vec3 startpos, NavPoint* currentpoint)
{
	point->fcost = glm::distance(point->pos, endpoint);
	//const float fcost = glm::distance(point->pos, startpos);
	point->gcost = currentpoint->GetNavCost() + glm::distance(currentpoint->pos, point->pos);
}

NavPoint* Getlowest(std::vector<NavPoint*> & points)
{
	if (points.size() == 0)
	{
		return nullptr;
	}
	NavPoint* Lowestnode = points[0];
	for (int i = 0; i < points.size(); i++)
	{
		if (Lowestnode->GetNavCost() > points[i]->GetNavCost())
		{
			Lowestnode = points[i];
		}
	}
	return Lowestnode;
}

bool AddToClosed(NavPoint* ppoint, NavTriangle* endtri)
{
	for (int i = 0; i < 3; i++)
	{
		if (ppoint->pos == endtri->Positons[i])
		{
			return true;
		}
	}
	return false;
}

bool Contains(NavPoint* point, std::vector<NavPoint*> & points, int *index)
{
	for (int i = 0; i < points.size(); i++)
	{
		if (*points[i] == *point)
		{
			*index = i;
			return true;
		}
	}
	return false;
}

void RemoveItem(NavPoint* point, std::vector<NavPoint*> & points)
{
	for (int i = 0; i < points.size(); i++)
	{
		if (points[i] == point)
		{
			points.erase(points.begin() + i);
			return;
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

ENavRequestStatus::Type NavigationMesh::CalculatePath_DSTAR_LTE(glm::vec3 Startpoint, glm::vec3 EndPos, NavigationPath** outpath)
{
	return ENavRequestStatus::Failed;
}

ENavRequestStatus::Type NavigationMesh::CalculatePath_DSTAR_BoardPhase(glm::vec3 Startpoint, glm::vec3 EndPos, NavigationPath** outpath)
{
	return ENavRequestStatus::Failed;
}

ENavRequestStatus::Type NavigationMesh::CalculatePath_ASTAR(glm::vec3 Startpoint, glm::vec3 EndPos, NavigationPath** outpath)
{
	NavigationPath* outputPath = new NavigationPath();
	*outpath = outputPath;
	NavTriangle* StartTri = FindTriangleFromWorldPos(Startpoint);
	if (StartTri == nullptr)
	{
		return ENavRequestStatus::FailedPointOffNavMesh;
	}
	NavTriangle* EndTri = FindTriangleFromWorldPos(EndPos);
	if (EndTri == nullptr)
	{
		return ENavRequestStatus::FailedPointOffNavMesh;
	}
	DebugLineDrawer::instance->AddLine(StartTri->avgcentre, StartTri->avgcentre + glm::vec3(0, 10, 0), glm::vec3(0, 1, 1), 100);
	DebugLineDrawer::instance->AddLine(EndTri->avgcentre, EndTri->avgcentre + glm::vec3(0, 10, 0), glm::vec3(0, 1, 1), 100);

	if (StartTri == EndTri)
	{
		//we are within a nav triangle so we path straight to the point 
		outputPath->Positions.push_back(EndPos);
	}

	std::vector<NavPoint*> ClosedList;
	std::vector<NavPoint*> OpenList;
	NavPoint* CurrentPoint = new NavPoint(StartTri->Positons[0]);
	CurrentPoint->owner = StartTri;
	OpenList.push_back(CurrentPoint);
	while (OpenList.size() > 0)
	{
		CurrentPoint = Getlowest(OpenList);
		RemoveItem(CurrentPoint, OpenList);
		if (AddToClosed(CurrentPoint, EndTri))
		{
			ClosedList.push_back(CurrentPoint);
			break;//path found
		}
		else
		{
			ClosedList.push_back(CurrentPoint);
		}
		for (int i = 0; i < CurrentPoint->owner->NearTriangles.size(); i++)
		{
			for (int x = 0; x < 3; x++)
			{
				NavPoint* newpoint = new NavPoint(CurrentPoint->owner->NearTriangles[i]->Positons[x]);
				newpoint->owner = CurrentPoint->owner->NearTriangles[i];
				int index = 0;
				if (!Contains(newpoint, ClosedList, &index))
				{
					CalulateCost(newpoint, EndPos, Startpoint, CurrentPoint);
					if (Contains(newpoint, OpenList, &index))
					{
						if (OpenList[index]->gcost > newpoint->gcost)
						{
							OpenList[index] = newpoint;
							newpoint->Parent = CurrentPoint;
							CalulateCost(OpenList[index], EndPos, Startpoint, CurrentPoint);
						}
					}
					else
					{
						newpoint->Parent = CurrentPoint;
						OpenList.push_back(newpoint);
					}
				}
			}
		}
	}
	outputPath->Positions.push_back(Startpoint);
	while (CurrentPoint->Parent != nullptr)
	{
		outputPath->Positions.push_back(CurrentPoint->pos);
		CurrentPoint = CurrentPoint->Parent;
	}
	outputPath->Positions.push_back(EndPos);
	for (int i = 0; i < outputPath->Positions.size(); i++)
	{
		if (i < outputPath->Positions.size() - 1 && DebugLineDrawer::instance != nullptr)
		{
			DebugLineDrawer::instance->AddLine(outputPath->Positions[i], outputPath->Positions[i + 1], glm::vec3(0, 1, 0), 100);
		}
	}
	/*for (int i = 0; i < ClosedList.size(); i++)
	{

		if (i < ClosedList.size() - 1 && DebugLineDrawer::instance != nullptr)
		{
			DebugLineDrawer::instance->AddLine(ClosedList[i]->pos, ClosedList[i + 1]->pos, glm::vec3(0, 1, 0), 100);
		}
	}*/
	//DebugLineDrawer::instance->AddLine(ClosedList[ClosedList.size() - 1]->pos, EndPos, glm::vec3(0, 0.2, 0), 100);



	DebugLineDrawer::instance->AddLine(Startpoint, Startpoint + glm::vec3(0, 10, 0), glm::vec3(0, 0, 1), 100);
	DebugLineDrawer::instance->AddLine(EndPos, EndPos + glm::vec3(0, 10, 0), glm::vec3(0, 0, 0.5), 100);
	return ENavRequestStatus::Complete;
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

float side(glm::vec2 v1, glm::vec2  v2, glm::vec2 point)
{
	return (v2.y - v1.y)*(point.x - v1.x) + (-v2.x + v1.x)*(point.y - v1.y);
}

bool pointInTriangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 point)
{
	bool checkSide1 = side(v1.xz, v2.xz, point.xz) >= 0;
	bool checkSide2 = side(v2.xz, v3.xz, point.xz) >= 0;
	bool checkSide3 = side(v3.xz, v1.xz, point.xz) >= 0;
	return checkSide1 && checkSide2 && checkSide3;
}

bool NavTriangle::IsPointInsideTri(glm::vec3 point)
{
	return pointInTriangle(Positons[0], Positons[1], Positons[2], point);
}
