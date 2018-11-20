
#include "NavigationMesh.h"
#include "Core/Assets/MeshLoader.h"
#include "Rendering/Core/DebugLineDrawer.h"
#include "Core/Assets/AssetManager.h"
#include "AI/Core/AISystem.h"
#include "AI/Core/Navigation/NavigationObstacle.h"
#include "Core/Platform/PlatformCore.h"
#include "AI/Generation/NavMeshGenerator.h"
#include "Core/Utils/DebugDrawers.h"
#include "glm/gtx/compatibility.hpp"

void NavigationMesh::SetupGrid()
{
	for (int x = 0; x < Gsize; x++)
	{
		for (int y = 0; y < Gsize; y++)
		{
			grid[x][y].Point = glm::vec2(x, y);
			if (x < Gsize - 1)
			{
				grid[x][y].NearNodes.push_back(&grid[x + 1][y]);
			}
			if (y < Gsize - 1)
			{
				grid[x][y].NearNodes.push_back(&grid[x][y + 1]);
			}
			if (y > 0)
			{
				grid[x][y].NearNodes.push_back(&grid[x][y - 1]);
			}
			if (x > 0)
			{
				grid[x][y].NearNodes.push_back(&grid[x - 1][y]);
			}
		}
	}
}
NavigationMesh::NavigationMesh()
{
	SetupGrid();
}

NavigationMesh::~NavigationMesh()
{}

float NavigationMesh::heuristic(DLTENode sFrom, DLTENode sTo)
{
	float distance{ 0 };
	sFrom.Point.x > sTo.Point.x ? distance = sFrom.Point.x - sTo.Point.x : distance = sTo.Point.x - sFrom.Point.x;
	sFrom.Point.y > sTo.Point.y ? distance += sFrom.Point.y - sTo.Point.y : distance += sTo.Point.y - sFrom.Point.y;
	return distance;
}

float * NavigationMesh::calculate_keys(DLTENode sTo, DLTENode * sFromPointer)
{
	float heuristicValue{ heuristic(*sFromPointer, sTo) };
	sFromPointer->rhs <= sFromPointer->g ? sFromPointer->key[0] = sFromPointer->rhs + heuristicValue + kM : sFromPointer->key[0] = sFromPointer->g + heuristicValue + kM;
	sFromPointer->rhs <= sFromPointer->g ? sFromPointer->key[1] = sFromPointer->rhs : sFromPointer->key[1] = sFromPointer->g;
	return sFromPointer->key;
}

bool key_less_than_key(float* keyFrom, float* keyTo)
{
	if (keyFrom[0] < keyTo[0])
	{
		return true;
	}
	else if (keyFrom[0] == keyTo[0] && keyFrom[1] < keyTo[1])
	{
		return true;
	}
	return false;
}
float * NavigationMesh::queue_top_key()
{
	if (!queue.empty())
	{
		return queue.front()->key;
	}
	else
	{
		return emptyState->key;
	}
}
DLTENode* NavigationMesh::queue_pop()
{
	DLTENode* temporaryStatePointer{ queue.front() };
	queue.pop_front();
	return temporaryStatePointer;

}
void NavigationMesh::queue_insert(DLTENode* statePointer)
{
	if (!queue.empty())
	{
		DLTENode temporaryState;
		for (size_t i = 0; i != queue.size(); ++i)
		{
			temporaryState = *(queue[i]);
			if (key_less_than_key(statePointer->key, temporaryState.key))
			{
				queue.insert(queue.begin() + i, statePointer);
				return;
			}
		}
		queue.push_back(statePointer);
	}
	else
	{
		queue.push_front(statePointer);
	}

}
void NavigationMesh::queue_remove(DLTENode s)
{
	if (!queue.empty())
	{
		DLTENode temporaryState;
		for (size_t i = 0; i < queue.size(); ++i)
		{
			temporaryState = *(queue[i]);
			if (s.Point.x == temporaryState.Point.x && s.Point.y == temporaryState.Point.y)
			{
				queue.erase(queue.begin() + i);
			}
		}
	}
}
DLTENode NavigationMesh::get_start()
{
	return *startnode;
}
DLTENode NavigationMesh::get_goal()
{
	return *goalnode;
}
void NavigationMesh::Reset()
{
	for (int i = 0; i < Plane->NavPoints.size(); i++)
	{
		Plane->NavPoints[i]->Reset();
	}
}
void NavigationMesh::run(std::vector<glm::vec3>& path)
{
	Reset();
	GridLTE();
	//std::vector<glm::vec3> path;
	while (get_start().Point.x != get_goal().Point.x || get_start().Point.y != get_goal().Point.y)
	{
		std::deque<DLTENode*> temporaryDeque{ neighbors(get_start()) };
		DLTENode* temporaryState{ temporaryDeque[0] };
		DLTENode* secondTemporaryState;
		for (size_t i = 1; i != temporaryDeque.size(); ++i)
		{
			secondTemporaryState = temporaryDeque[i];
			int temp_cost{ traversal_cost(get_start(), *temporaryState) };
			int temp_cost2{ traversal_cost(get_start(), *secondTemporaryState) };
			if (secondTemporaryState->g != MAX_int)
			{
				if (temporaryState->g == MAX_int || secondTemporaryState->g + temp_cost2 < temporaryState->g + temp_cost)
				{
					temporaryState = secondTemporaryState;
				}
			}
		}
		/* if start->g == numeric_limits<int>::max() then there is no path */
		if (get_start().g == FloatMAX)
		{
			//			__debugbreak();
			break;
		}
		path.push_back(glm::vec3(startnode->Point.x, 1, startnode->Point.y));
		startnode = (temporaryState);
		//GridLTE();
		//	std::cout << dStarLite.get_start().x << " " << dStarLite.get_start().y << std::endl;
			/* if any edge cost has changed */
			//if (changesDetected)
			//{
			//	dStarLite.set_k_m(dStarLite.get_k_m() + dStarLite.heuristic(lastState, dStarLite.get_start()));
			//	lastState = dStarLite.get_start();
			//	// for all directed changes (u, v), update_cost (u, v) and then update_state(v)
			//	dStarLite.update_cost(4, dStarLite.get_state_pointer(1, 2), *(dStarLite.get_state_pointer(1, 3)));
			//	dStarLite.update_state(dStarLite.get_state_pointer(1, 3));
			//	// end for all
			//	dStarLite.compute_shortest_path();
			//	changesDetected = false;
			//}

	}
#if 0
	for (int i = 0; i < path.size(); i++)
	{
		DebugLineDrawer::Get()->AddLine(path[i], path[i] + glm::vec3(0, 2, 0), glm::vec3(0, 0, 1), 100);
	}
#endif
}

void NavigationMesh::SetTarget(glm::vec3 Target, glm::vec3 Origin)
{
#if 0
	float CurrentPoint = FloatMAX;
	for (int i = 0; i < Plane->NavPoints.size(); i++)
	{
		const float newdist = glm::distance2(Plane->NavPoints[i]->GetPos(Plane), Origin);
		if (newdist < CurrentPoint)
		{
			CurrentPoint = newdist;
			startnode = Plane->NavPoints[i];
		}
	}
	CurrentPoint = FloatMAX;
	for (int i = 0; i < Plane->NavPoints.size(); i++)
	{
		const float newdist = glm::distance2(Plane->NavPoints[i]->GetPos(Plane), Target);
		if (newdist < CurrentPoint)
		{
			CurrentPoint = newdist;
			goalnode = Plane->NavPoints[i];
		}
	}
#endif
	Plane->ResolvePositionToNode(Target, &goalnode);
	Plane->ResolvePositionToNode(Origin, &startnode);
	//DebugDrawers::DrawDebugSphere(Origin, 2, glm::vec3(1), 16, false, 100000);
	Origin.y = -10.0f;
	DebugDrawers::DrawDebugSphere(Origin, 0.25f, glm::vec3(0.5f), 16, false, 1);
}

void NavigationMesh::GridLTE()
{
	//init;
	if (startnode == nullptr)
	{
		startnode = &grid[10][10];
		goalnode = &grid[30][19];
	}
	//DebugLineDrawer::Get()->AddLine(glm::vec3(startnode->Point.x, 0, startnode->Point.y), glm::vec3(startnode->Point.x, 10, startnode->Point.y), glm::vec3(0, 1, 1), 100);
	queue_insert(goalnode);
	startnode->g = FloatMAX;
	startnode->rhs = FloatMAX;
	goalnode->g = FloatMAX;
	goalnode->rhs = 0;
	calculate_keys(*startnode, goalnode);

	emptyState = new DLTENode();
	emptyState->key[0] = FloatMAX;
	emptyState->key[1] = FloatMAX;
	DLTENode* ptr = nullptr;
	while (key_less_than_key(queue_top_key(), calculate_keys(*startnode, startnode)) || startnode->rhs != startnode->g)
	{
		ptr = queue_pop();
		//DebugDrawers::DrawDebugLine(ptr->GetPos(Plane), ptr->GetPos(Plane) + glm::vec3(0, 2, 0), glm::vec3(1, 0, 0), false, 1000);
		ensure(ptr);
		if (key_less_than_key(queue_top_key(), calculate_keys(*startnode, ptr)))
		{
			queue_insert(ptr);//possible path node 
		}
		else
		{
			std::deque<DLTENode*> temporaryDeque{ neighbors(*ptr) };
			if (ptr->g > ptr->rhs)
			{
				ptr->g = ptr->rhs;
				for (size_t i = 0; i < temporaryDeque.size(); i++)
				{
					update_state(temporaryDeque[i]);
				}
			}
			else
			{
				ptr->g = FloatMAX;
				for (size_t i = 0; i < temporaryDeque.size(); i++)
				{
					update_state(temporaryDeque[i]);
				}
				update_state(ptr);
			}
		}
	}
	//ensure(queue.size());
//	DebugLineDrawer::Get()->AddLine(glm::vec3(goalnode->Point.x, 0, goalnode->Point.y), glm::vec3(goalnode->Point.x, 10, goalnode->Point.y), glm::vec3(0, 1, 1), 100);

}

std::deque<DLTENode*> NavigationMesh::neighbors(DLTENode s)
{
	std::deque<DLTENode*> temporaryDeque;
#if 0
	DLTENode* temporaryStatePointer;
	std::string coordinates;
	int temporaryX;
	int temporaryY;
	for (size_t i = 0; i != DIRECTIONS_WIDTH; ++i)
	{
		temporaryX = s.Point.x + DIRECTIONS[i][0];
		temporaryY = s.Point.y + DIRECTIONS[i][1];
		//coordinates = coordinates_to_string(temporaryX, temporaryY);
		/*if (grid.find(coordinates) == grid.end())
		{
			temporaryStatePointer = new state();
			temporaryStatePointer->x = temporaryX;
			temporaryStatePointer->y = temporaryY;
			grid[coordinates] = temporaryStatePointer;
		}
		else
		{
			temporaryStatePointer = grid[coordinates];
		}*/
		//todo: link the navmesh graph here
		//search though the current nodes connections which will be assigned at bake time
		temporaryStatePointer = &grid[temporaryX][temporaryY];
		if (temporaryY >= 0 && temporaryX >= 0)
		{
			ensure(temporaryY >= 0);
			ensure(temporaryX >= 0);
			temporaryDeque.push_back(temporaryStatePointer);
		}

	}
#else
	for (int i = 0; i < s.NearNodes.size(); i++)
	{
		temporaryDeque.push_back(s.NearNodes[i]);
	}
#endif
	DebugEnsure(temporaryDeque.size());
	return temporaryDeque;
}
int NavigationMesh::traversal_cost(DLTENode sFrom, DLTENode sTo)
{
	int edgeCost{ 1 };
	for (size_t i = 0; i != DIRECTIONS_WIDTH; ++i)
	{
		if (sFrom.Point.x + DIRECTIONS[i][0] == sTo.Point.x && sFrom.Point.y + DIRECTIONS[i][1] == sTo.Point.y)
		{
			edgeCost = sFrom.edgeCost[i];
			break;
		}
	}
	return edgeCost;
}

void NavigationMesh::update_state(DLTENode* statePointer)
{
	if (statePointer->Point.x != goalnode->Point.x || statePointer->Point.y != goalnode->Point.y)
	{
		std::deque<DLTENode*> temporaryDeque = neighbors(*statePointer);
		DLTENode* temporaryStatePointer = temporaryDeque[0];
		if (temporaryStatePointer->g != MAX_int)
		{
			statePointer->rhs = temporaryStatePointer->g + traversal_cost(*statePointer, *temporaryStatePointer);
		}
		int temp_cost;
		for (size_t i = 1; i < temporaryDeque.size(); ++i)
		{
			temporaryStatePointer = temporaryDeque[i];
			temp_cost = traversal_cost(*statePointer, *temporaryStatePointer);
			if (temporaryStatePointer->g != MAX_int && statePointer->rhs > temporaryStatePointer->g + temp_cost)
			{
				statePointer->rhs = temporaryStatePointer->g + temp_cost;
			}
		}
	}
	// if s is in queue, then remove
	queue_remove(*statePointer);

	if (statePointer->g != statePointer->rhs)
	{
		calculate_keys(*startnode, statePointer);
		//queue_insert(statePointer);
		queue_insert(statePointer);
	}
}

void NavigationMesh::RenderGrid()
{
	for (int x = 0; x < Gsize; x++)
	{
		for (int y = 0; y < Gsize; y++)
		{
			DebugLineDrawer::Get()->AddLine(glm::vec3(x, 0, y), glm::vec3(x, 0.5, y), grid[x][y].Blocked ? glm::vec3(1, 0, 0) : glm::vec3(0, 1, 0), 100);
		}
	}
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

void NavigationMesh::GenTestMesh()
{
	return;
	SetTarget(glm::vec3(50.5, 0, 0), glm::vec3(60, 0, 10));
	run(std::vector<glm::vec3>());
	SetTarget(glm::vec3(50.5, 0, -10), glm::vec3(70, 0, -10));
	run(std::vector<glm::vec3>());
}

void NavigationMesh::DrawNavMeshLines()
{
	glm::vec3 offset = glm::vec3(0, 0.2f, 0);
	for (int i = 0; i < Triangles.size(); i++)
	{
		for (int x = 0; x < 3; x++)
		{
			const int next = (x + 1) % 3;
			DebugDrawers::DrawDebugLine(offset + Triangles[i]->Positons[x], offset + Triangles[i]->Positons[next], glm::vec3(1, 1, 1));
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
	point->hcost = glm::distance(point->pos, endpoint);
	//const float fcost = glm::distance(point->pos, startpos);
	point->gcost = currentpoint->gcost + glm::distance(currentpoint->pos, point->pos);
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

ENavRequestStatus::Type NavigationMesh::CalculatePath_DSTAR_BoardPhase(glm::vec3 Startpoint, glm::vec3 EndPos, NavigationPath** outpath)
{
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
	NavTriangle* StartTri = FindTriangleFromWorldPos(Startpoint);
	if (StartTri == nullptr)
	{
		//return ENavRequestStatus::FailedPointOffNavMesh;
	}
	NavTriangle* EndTri = FindTriangleFromWorldPos(EndPos);
	if (EndTri == nullptr)
	{
		//return ENavRequestStatus::FailedPointOffNavMesh;
	}
	//DebugLineDrawer::Get()->AddLine(StartTri->avgcentre, StartTri->avgcentre + glm::vec3(0, 10, 0), glm::vec3(0, 1, 1), 100);
	//DebugLineDrawer::Get()->AddLine(EndTri->avgcentre, EndTri->avgcentre + glm::vec3(0, 10, 0), glm::vec3(0, 1, 1), 100);

	if (StartTri == EndTri)
	{
		//we are within a nav triangle so we path straight to the point 
	//	outputPath->Positions.push_back(EndPos);
	}
	SetTarget(EndPos, Startpoint);
	run(outputPath->Positions);

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

void NavigationMesh::ConstructPath(NavigationPath* outputPath, glm::vec3 Startpoint, NavPoint* CurrentPoint, glm::vec3 EndPos)
{
	outputPath->Positions.push_back(EndPos);
	if (CurrentPoint != nullptr)
	{
		while (CurrentPoint->Parent != nullptr)
		{
			outputPath->Positions.push_back(CurrentPoint->pos);
			CurrentPoint = CurrentPoint->Parent;
		}
	}
	outputPath->Positions.push_back(Startpoint);
	for (int i = 0; i < outputPath->Positions.size(); i++)
	{
		if (i < outputPath->Positions.size() - 1 && DebugLineDrawer::Get() != nullptr)
		{
			DebugLineDrawer::Get()->AddLine(outputPath->Positions[i], outputPath->Positions[i + 1], glm::vec3(0, 1, 0), 100);
		}
	}
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
	DebugLineDrawer::Get()->AddLine(StartTri->avgcentre, StartTri->avgcentre + glm::vec3(0, 10, 0), glm::vec3(0, 1, 1), 100);
	DebugLineDrawer::Get()->AddLine(EndTri->avgcentre, EndTri->avgcentre + glm::vec3(0, 10, 0), glm::vec3(0, 1, 1), 100);

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
	CalulateCost(CurrentPoint, EndPos, Startpoint, CurrentPoint);
	DebugLineDrawer::Get()->AddLine(CurrentPoint->pos, CurrentPoint->pos + glm::vec3(0, 10, 0), glm::vec3(0, 1, 0), 100);
	while (OpenList.size() > 0)
	{
		CurrentPoint = Getlowest(OpenList);
		RemoveItem(CurrentPoint, OpenList);
		if (AddToClosed(CurrentPoint, EndTri))
		{
			//	ClosedList.push_back(CurrentPoint);
			DebugLineDrawer::Get()->AddLine(CurrentPoint->pos, CurrentPoint->pos + glm::vec3(0, 10, 0), glm::vec3(0, 1, 0), 100);
			break;//path found
		}
		else
		{
			DebugLineDrawer::Get()->AddLine(CurrentPoint->pos, CurrentPoint->pos + glm::vec3(0, 10, 0), glm::vec3(1, 0, 0), 100);
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
						if (OpenList[index]->GetNavCost() > newpoint->GetNavCost())
						{
							DebugLineDrawer::Get()->AddLine(OpenList[index]->pos, OpenList[index]->pos + glm::vec3(0, 10, 0), glm::vec3(0, 1, 0), 100);
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
	ConstructPath(outputPath, Startpoint, CurrentPoint, EndPos);
	DebugLineDrawer::Get()->AddLine(Startpoint, Startpoint + glm::vec3(0, 10, 0), glm::vec3(0, 0, 1), 100);
	DebugLineDrawer::Get()->AddLine(EndPos, EndPos + glm::vec3(0, 10, 0), glm::vec3(0, 0, 0.2), 100);
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

glm::vec3 DLTENode::GetPos(NavPlane* p)
{
	return glm::vec3(Point.x, 10 + p->ZHeight, Point.y);
}
