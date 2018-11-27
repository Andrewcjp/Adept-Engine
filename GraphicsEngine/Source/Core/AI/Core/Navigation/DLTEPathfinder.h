#pragma once
#include "Core/Utils/MathUtils.h"
struct DLTENode;
class DLTEQueue;
struct NavPlane;
class DLTEPathfinder
{
public:
	DLTEPathfinder();
	~DLTEPathfinder();
	void SetTarget(glm::vec3 Target, glm::vec3 Origin);
	void Execute(std::vector<glm::vec3>& path);
	NavPlane* Plane = nullptr;
private:
	float Heuristic(DLTENode sFrom, DLTENode sTo);
	float * ComputeKeys(DLTENode sTo, DLTENode * sFromPointer);
	DLTENode get_start();
	DLTENode get_goal();
	void Reset();
	void GridLTE();
	std::deque<DLTENode*> neighbors(DLTENode s);
	int ComputeCost(DLTENode sFrom, DLTENode sTo);
	void UpdateState(DLTENode * statePointer);
	int kM = 0;
	DLTENode* goalnode = nullptr;
	DLTENode* startnode = nullptr;
	DLTEQueue* Queue = nullptr;
};

const size_t DIRECTIONS_WIDTH = 8;
const size_t DIRECTIONS_HEIGHT = 2;
const int DIRECTIONS[DIRECTIONS_WIDTH][DIRECTIONS_HEIGHT] = { { 0, 1 },{ 0, -1 },{ 1, 0 },{ -1, 0 },{ 1, 1 },{ 1, -1 },{ -1, 1 },{ -1, -1 } };
class NavigationObstacle;
struct NavPlane;
struct Tri;
struct DLTENode
{
	DLTENode()
	{
		Reset();
	}
	DLTENode(glm::vec3 pos)
	{
		Point.x = pos.x;
		Point.y = pos.z;
		Reset();
	}
	void Reset()
	{
		g = MathUtils::FloatMAX;
		rhs = MathUtils::FloatMAX;
		key[0] = 0.0f;
		key[1] = 0.0f;
	}
	glm::vec3 GetPos(NavPlane* p);
	float key[2];
	float g = MathUtils::FloatMAX;
	float rhs = MathUtils::FloatMAX;
	bool Blocked = false;
	glm::vec2 Point = glm::vec2(0, 0);
	int edgeCost[DIRECTIONS_WIDTH] = { 1, 1, 1, 1, 1, 1, 1, 1 };
	std::vector<DLTENode*> NearNodes;
	Tri* OwnerTri = nullptr;
};
class DLTEQueue
{
public:
	DLTEQueue();
	~DLTEQueue();
	float * TopKey();
	DLTENode * Pop();
	void Insert(DLTENode * statePointer);
	void Remove(DLTENode s);
private:
	std::deque<DLTENode*> queue;
	DLTENode* emptyState = nullptr;
};

