#include "Source/Core/Stdafx.h"
#include "DLTEPathfinder.h"
#include "AI/Generation/NavMeshGenerator.h"
#include "Core/Utils/DebugDrawers.h"
#include "Core/Platform/PlatformCore.h"
#include "Core/Platform/Logger.h"

DLTEPathfinder::DLTEPathfinder()
{
	Queue = new DLTEQueue();
}

DLTEPathfinder::~DLTEPathfinder()
{
	SafeDelete(Queue);
}

float DLTEPathfinder::Heuristic(DLTENode sFrom, DLTENode sTo)
{
	float distance = 0.0f;
	sFrom.Point.x > sTo.Point.x ? distance = sFrom.Point.x - sTo.Point.x : distance = sTo.Point.x - sFrom.Point.x;
	sFrom.Point.y > sTo.Point.y ? distance += sFrom.Point.y - sTo.Point.y : distance += sTo.Point.y - sFrom.Point.y;
	return distance;
}

float * DLTEPathfinder::ComputeKeys(DLTENode sTo, DLTENode * sFromPointer)
{
	float heuristicValue = Heuristic(*sFromPointer, sTo);
	sFromPointer->rhs <= sFromPointer->g ? sFromPointer->key[0] = sFromPointer->rhs + heuristicValue + kM : sFromPointer->key[0] = sFromPointer->g + heuristicValue + kM;
	sFromPointer->rhs <= sFromPointer->g ? sFromPointer->key[1] = sFromPointer->rhs : sFromPointer->key[1] = sFromPointer->g;
	return sFromPointer->key;
}

bool KeyLessThan(float* keyFrom, float* keyTo)
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

DLTENode DLTEPathfinder::get_start()
{
	return *startnode;
}

DLTENode DLTEPathfinder::get_goal()
{
	return *goalnode;
}

void DLTEPathfinder::Reset()
{
	for (int i = 0; i < Plane->NavPoints.size(); i++)
	{
		Plane->NavPoints[i]->Reset();
	}
	Queue->Clear();
}

void DLTEPathfinder::Execute(std::vector<glm::vec3>& path)
{
	Reset();
	GridLTE();
	while (get_start().Point.x != get_goal().Point.x || get_start().Point.y != get_goal().Point.y)
	{
		std::deque<DLTENode*> temporaryDeque = neighbors(get_start());
		DLTENode* temporaryState = temporaryDeque[0];//there should always be a node returned
		DLTENode* secondTemporaryState;
		for (size_t i = 1; i != temporaryDeque.size(); ++i)
		{
			secondTemporaryState = temporaryDeque[i];
			int temp_cost{ ComputeCost(get_start(), *temporaryState) };
			int temp_cost2{ ComputeCost(get_start(), *secondTemporaryState) };
			if (secondTemporaryState->g != MathUtils::MAX_int)
			{
				if (temporaryState->g == MathUtils::MAX_int || secondTemporaryState->g + temp_cost2 < temporaryState->g + temp_cost)
				{
					temporaryState = secondTemporaryState;
				}
			}
		}
		/* if get_start().g == Max float then there is no path */
		if (get_start().g == MathUtils::FloatMAX)
		{
			break;
		}
		path.push_back(glm::vec3(startnode->Point.x, 1, startnode->Point.y));
		startnode = temporaryState;
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
}

void DLTEPathfinder::SetTarget(glm::vec3 Target, glm::vec3 Origin)
{
	Plane->ResolvePositionToNode(Target, &goalnode);
	Plane->ResolvePositionToNode(Origin, &startnode);
	Origin.y = -10.0f;
	DebugDrawers::DrawDebugSphere(Origin, 0.25f, glm::vec3(0.5f), 16, false, 1);
}

void DLTEPathfinder::GridLTE()
{
	Queue->Insert(goalnode);
	startnode->g = MathUtils::FloatMAX;
	startnode->rhs = MathUtils::FloatMAX;
	goalnode->g = MathUtils::FloatMAX;
	goalnode->rhs = 0;
	ComputeKeys(*startnode, goalnode);
	DLTENode* ptr = nullptr;
	while (KeyLessThan(Queue->TopKey(), ComputeKeys(*startnode, startnode)) || startnode->rhs != startnode->g)
	{
		ptr = Queue->Pop();
		ensure(ptr);
		if (KeyLessThan(Queue->TopKey(), ComputeKeys(*startnode, ptr)))
		{
			Queue->Insert(ptr);//possible path node 
		}
		else
		{
			std::deque<DLTENode*> temporaryDeque{ neighbors(*ptr) };
			if (ptr->g > ptr->rhs)
			{
				ptr->g = ptr->rhs;
				for (size_t i = 0; i < temporaryDeque.size(); i++)
				{
					UpdateState(temporaryDeque[i]);
				}
			}
			else
			{
				ptr->g = MathUtils::FloatMAX;
				for (size_t i = 0; i < temporaryDeque.size(); i++)
				{
					UpdateState(temporaryDeque[i]);
				}
				UpdateState(ptr);
			}
		}
	}
}

std::deque<DLTENode*> DLTEPathfinder::neighbors(DLTENode s)
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
int DLTEPathfinder::ComputeCost(DLTENode sFrom, DLTENode sTo)
{
	int edgeCost = 1;
	for (size_t i = 0; i < DIRECTIONS_WIDTH; ++i)
	{
		if (sFrom.Point.x + DIRECTIONS[i][0] == sTo.Point.x && sFrom.Point.y + DIRECTIONS[i][1] == sTo.Point.y)
		{
			edgeCost = sFrom.edgeCost[i];
			break;
		}
	}
	return edgeCost;
}

void DLTEPathfinder::UpdateState(DLTENode* statePointer)
{
	if (statePointer->Point.x != goalnode->Point.x || statePointer->Point.y != goalnode->Point.y)
	{
		std::deque<DLTENode*> temporaryDeque = neighbors(*statePointer);
		DLTENode* temporaryStatePointer = temporaryDeque[0];
		if (temporaryStatePointer->g != MathUtils::MAX_int)
		{
			statePointer->rhs = temporaryStatePointer->g + ComputeCost(*statePointer, *temporaryStatePointer);
		}
		int temp_cost;
		for (size_t i = 1; i < temporaryDeque.size(); ++i)
		{
			temporaryStatePointer = temporaryDeque[i];
			temp_cost = ComputeCost(*statePointer, *temporaryStatePointer);
			if (temporaryStatePointer->g != MathUtils::MAX_int && statePointer->rhs > temporaryStatePointer->g + temp_cost)
			{
				statePointer->rhs = temporaryStatePointer->g + temp_cost;
			}
		}
	}
	// if s is in queue, then remove
	Queue->Remove(*statePointer);
	if (statePointer->g != statePointer->rhs)
	{
		ComputeKeys(*startnode, statePointer);
		Queue->Insert(statePointer);
	}
}


glm::vec3 DLTENode::GetPos(NavPlane* p)
{
	return glm::vec3(Point.x, 10 + p->ZHeight, Point.y);
}

DLTEQueue::DLTEQueue()
{
	emptyState = new DLTENode();
	emptyState->key[0] = MathUtils::FloatMAX;
	emptyState->key[1] = MathUtils::FloatMAX;
}

DLTEQueue::~DLTEQueue()
{
	SafeDelete(emptyState);
}

float * DLTEQueue::TopKey()
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

DLTENode * DLTEQueue::Pop()
{
	DLTENode* temporaryStatePointer = queue.front();
	queue.pop_front();
	return temporaryStatePointer;
}

void DLTEQueue::Insert(DLTENode * statePointer)
{
	if (!queue.empty())
	{
		DLTENode temporaryState;
		for (size_t i = 0; i != queue.size(); ++i)
		{
			temporaryState = *(queue[i]);
			if (KeyLessThan(statePointer->key, temporaryState.key))
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

void DLTEQueue::Remove(DLTENode s)
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

void DLTEQueue::Clear()
{
	queue.clear();
}
