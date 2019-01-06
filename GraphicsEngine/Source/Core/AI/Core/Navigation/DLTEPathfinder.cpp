#include "DLTEPathfinder.h"
#include "AI/Generation/NavMeshGenerator.h"
#include "Core/Utils/DebugDrawers.h"
#include "Core/Platform/PlatformCore.h"
#include "Core/Platform/Logger.h"
#include "AI/Core/AISystem.h"

DLTEPathfinder::DLTEPathfinder()
{
	Queue = new DLTEQueue();
}

DLTEPathfinder::~DLTEPathfinder()
{
	SafeDelete(Queue);
}

float DLTEPathfinder::Heuristic(const DLTENode* sFrom, const DLTENode* sTo)
{
	float distance = glm::distance(sFrom->Point, sTo->Point);
	distance += fmaxf(sFrom->TraversalCost, sTo->TraversalCost);
	return distance;
}

float * DLTEPathfinder::ComputeKeys(const DLTENode* sTo, DLTENode * sFromPointer)
{
	float heuristicValue = Heuristic(sFromPointer, sTo);
	sFromPointer->key[0] = glm::min(sFromPointer->g, sFromPointer->rhs) + heuristicValue + kM;
	sFromPointer->key[1] = glm::min(sFromPointer->g, sFromPointer->rhs) + heuristicValue + kM;
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

DLTENode* DLTEPathfinder::get_start()
{
	return startnode;
}

DLTENode* DLTEPathfinder::get_goal()
{
	return goalnode;
}

void DLTEPathfinder::Reset()
{
	Plane->Reset();
	Queue->Clear();
}

bool DLTEPathfinder::Execute(std::vector<glm::vec3>& path)
{
	Reset();
	ComputeDLTE();
	DLTENode* LastState = get_start();
	while (get_start()->Point.x != get_goal()->Point.x || get_start()->Point.y != get_goal()->Point.y)
	{
		// if Start Node g cost is FloatMAX then there is no path so abort
		if (get_start()->g == MathUtils::FloatMAX)
		{
			return false;
		}
		std::deque<DLTENode*> temporaryDeque = GetNeighbors(get_start());
		DLTENode* temporaryState = temporaryDeque[0];//there should always be a node returned
		DLTENode* secondTemporaryState;
		for (size_t i = 1; i < temporaryDeque.size(); i++)
		{
			secondTemporaryState = temporaryDeque[i];
			int temp_cost = ComputeCost(get_start(), temporaryState);
			int temp_cost2 = ComputeCost(get_start(), secondTemporaryState);
			if (secondTemporaryState->g != MathUtils::MAX_int)
			{
				if (temporaryState->g == MathUtils::MAX_int || secondTemporaryState->g + temp_cost2 < temporaryState->g + temp_cost)
				{
					temporaryState = secondTemporaryState;
				}
			}
		}
		path.push_back(glm::vec3(startnode->Point.x, 1, startnode->Point.y));
		startnode = temporaryState;
		//todo: Finish Path Re planning
		if (Plane->MeshType == ENavMeshType::Dynamic && Plane->IsNavDirty)
		{				
			kM += Heuristic(LastState, startnode);
			//UpdateCost(4,)
			//Update the cost of nodes that have changed
			ComputeDLTE();
			Plane->IsNavDirty = false;
		}


	}
	return true;
}

void DLTEPathfinder::SetTarget(glm::vec3 Target, glm::vec3 Origin)
{
	Plane->ResolvePositionToNode(Target, &goalnode);
	Plane->ResolvePositionToNode(Origin, &startnode);
	if (AISystem::GetDebugMode() == EAIDebugMode::PathOnly || AISystem::GetDebugMode() == EAIDebugMode::All)
	{
		Origin.y = -10.0f;
		DebugDrawers::DrawDebugSphere(Origin, 0.25f, glm::vec3(0.5f), 16, false, 1);
	}
}

void DLTEPathfinder::ComputeDLTE()
{
	Queue->Insert(goalnode);
	startnode->g = MathUtils::FloatMAX;
	startnode->rhs = MathUtils::FloatMAX;
	goalnode->g = MathUtils::FloatMAX;
	goalnode->rhs = 0;
	ComputeKeys(startnode, goalnode);
	DLTENode* ptr = nullptr;
	while (KeyLessThan(Queue->TopKey(), ComputeKeys(startnode, startnode)) || startnode->rhs != startnode->g)
	{
		float* Kold = new float[2];
		Kold = Queue->TopKey();
		ptr = Queue->Pop();
		ensure(ptr);
		if (KeyLessThan(Kold, ComputeKeys(startnode, ptr)))
		{
			Queue->Insert(ptr);//possible path node 
		}
		else
		{
			std::deque<DLTENode*> temporaryDeque = GetNeighbors(ptr);
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

std::deque<DLTENode*> DLTEPathfinder::GetNeighbors(DLTENode* s)
{
	std::deque<DLTENode*> temporaryQueue;
	for (int i = 0; i < s->NearNodes.size(); i++)
	{
		temporaryQueue.push_back(s->NearNodes[i]);
	}
	//DebugEnsure(temporaryDeque.size());
	return temporaryQueue;
}

int DLTEPathfinder::ComputeCost(DLTENode* sFrom, DLTENode* sTo)
{
	int edgeCost = 1;
	for (size_t i = 0; i < DIRECTIONS_COUNT; ++i) //todo: re planning for dynamic Nav meshes
	{
	}
	return 1;
}
void DLTEPathfinder::UpdateCost(float cost, DLTENode* sFrom, DLTENode sTo)
{
	for (size_t i = 0; i < sFrom->NearNodes.size(); i++)
	{
		for (int i = 0; i < sFrom->NearNodes.size(); i++)
		{
			sFrom->edgeCost[i] = cost;
		}	
	}
}


void DLTEPathfinder::UpdateState(DLTENode* statePointer)
{
	if (statePointer->Point.x != goalnode->Point.x || statePointer->Point.y != goalnode->Point.y)
	{
		std::deque<DLTENode*> temporaryDeque = GetNeighbors(statePointer);
		DLTENode* temporaryStatePointer = temporaryDeque[0];
		if (temporaryStatePointer->g != MathUtils::MAX_int)
		{
			statePointer->rhs = temporaryStatePointer->g + ComputeCost(statePointer, temporaryStatePointer);
		}
		int min_cost = 0;//Find the min cost node
		for (size_t i = 1; i < temporaryDeque.size(); ++i)
		{
			temporaryStatePointer = temporaryDeque[i];
			min_cost = ComputeCost(statePointer, temporaryStatePointer);
			if (temporaryStatePointer->g != MathUtils::MAX_int && statePointer->rhs > temporaryStatePointer->g + min_cost)
			{
				statePointer->rhs = temporaryStatePointer->g + min_cost;
			}
		}
	}
	// if s is in queue, then remove
	Queue->Remove(*statePointer);
	if (statePointer->g != statePointer->rhs)
	{
		ComputeKeys(startnode, statePointer);
		Queue->Insert(statePointer);
	}
}


glm::vec3 DLTENode::GetPos(NavPlane* p)
{
	return glm::vec3(Point.x, /*10 + p->GetHeight()*/Point.z, Point.y);
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
