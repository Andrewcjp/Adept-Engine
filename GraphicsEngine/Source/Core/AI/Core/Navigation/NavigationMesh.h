#pragma once
struct NavTriangle
{
	glm::vec3 Positons[3];
	int Cost = 1;
	bool Traverable = true;
	std::vector<NavTriangle*> NearTriangles;
	int Id = 0;
	glm::vec3 avgcentre = glm::vec3();
	bool IsPointInsideTri(glm::vec3 point);
};
struct NavigationPath
{
	std::vector<glm::vec3> Positions;
};
struct NavPoint
{
	glm::vec3 pos;
	NavPoint(glm::vec3 ipos)
	{
		pos = ipos;
	}
	float GetNavCost()
	{
		return gcost + hcost;
	}
	float gcost = 0;
	float hcost = 0;
	NavTriangle* owner = nullptr;
	bool operator==(const NavPoint& rhs)
	{
		return rhs.pos == pos;
	}
	NavPoint* Parent = nullptr;
	glm::vec2 NavKey = glm::vec2();
};

struct NavNode
{
	NavNode(glm::vec3 npos)
	{
		Pos = npos;
	}
	glm::vec3 Pos = glm::vec3(0, 0, 0);
	std::vector<NavNode*> NearNodes;
};

namespace ENavRequestStatus
{
	enum Type
	{
		Failed,
		FailedPointOffNavMesh,
		Complete,
	};
}
const int MAX_int = std::numeric_limits<int>::max();
const float FloatMAX = std::numeric_limits<float>::max();
#if 1
const size_t DIRECTIONS_WIDTH = 8;
const size_t DIRECTIONS_HEIGHT = 2;
const int DIRECTIONS[DIRECTIONS_WIDTH][DIRECTIONS_HEIGHT] = { { 0, 1 },{ 0, -1 },{ 1, 0 },{ -1, 0 },{ 1, 1 },{ 1, -1 },{ -1, 1 },{ -1, -1 } };
#else
const size_t DIRECTIONS_WIDTH = 4;
const size_t DIRECTIONS_HEIGHT = 2;
const int DIRECTIONS[DIRECTIONS_WIDTH][DIRECTIONS_HEIGHT] = { { 0, 1 },{ 0, -1 },{ 1, 0 },{ -1, 0 } };
#endif
class NavigationObstacle;
struct NavPlane;
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
		g = FloatMAX;
		rhs = FloatMAX;
		key[0] = 0.0f;
		key[1] = 0.0f;
	}
	glm::vec3 GetPos(NavPlane* p);
	float key[2];
	float g = FloatMAX;
	float rhs = FloatMAX;
	bool Blocked = false;
	glm::vec2 Point = glm::vec2(0, 0);
	int edgeCost[DIRECTIONS_WIDTH] = { 1, 1, 1, 1, 1, 1, 1, 1 };
	std::vector<DLTENode*> NearNodes;
};
class NavigationMesh
{
public:

	void SetupGrid();
	NavigationMesh();
	~NavigationMesh();
	float heuristic(DLTENode sFrom, DLTENode sTo);
	float * calculate_keys(DLTENode sTo, DLTENode * sFromPointer);
	float * queue_top_key();
	DLTENode * queue_pop();
	void queue_insert(DLTENode * statePointer);
	void queue_remove(DLTENode s);
	NavigationMesh::DLTENode get_start();
	NavigationMesh::DLTENode get_goal();
	void Reset();
	void run(std::vector<glm::vec3>& path);
	void SetTarget(glm::vec3 Target, glm::vec3 Origin);
	void GridLTE();
	std::deque<DLTENode*> neighbors(DLTENode s);
	int traversal_cost(DLTENode sFrom, DLTENode sTo);
	void update_state(DLTENode * statePointer);
	void RenderGrid();
	void GenTestMesh();
	void DrawNavMeshLines();
	void PopulateNearLists();
	NavTriangle * FindTriangleFromWorldPos(glm::vec3 worldpos);

	ENavRequestStatus::Type CalculatePath(glm::vec3 Startpoint, glm::vec3 EndPos, NavigationPath** outputPath);

	void RegisterObstacle(NavigationObstacle* NewObstacle);
	void NotifyNavMeshUpdate();
	NavPlane* Plane = nullptr;
private:
	ENavRequestStatus::Type CalculatePath_DSTAR_LTE(glm::vec3 Startpoint, glm::vec3 EndPos, NavigationPath ** outpath);

	void ConstructPath(NavigationPath* outputPath, glm::vec3 Startpoint, NavPoint* CurrentPoint, glm::vec3 EndPos);

	ENavRequestStatus::Type CalculatePath_DSTAR_BoardPhase(glm::vec3 Startpoint, glm::vec3 EndPos, NavigationPath ** outpath);
	ENavRequestStatus::Type CalculatePath_ASTAR(glm::vec3 Startpoint, glm::vec3 EndPos, NavigationPath ** outpath);
	bool NavMeshNeedsUpdate = false;
	std::vector<NavTriangle*> Triangles;
	std::vector<NavigationObstacle*> Obstacles;

#define Gsize 50
	DLTENode grid[Gsize][Gsize];

	//dlte
	std::deque<DLTENode*> queue;
	DLTENode* emptyState = nullptr;
	int kM;
	DLTENode* goalnode = nullptr;
	DLTENode* startnode = nullptr;

};

