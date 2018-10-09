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
namespace ENavRequestStatus
{
	enum Type
	{
		Failed,
		FailedPointOffNavMesh,
		Complete,
	};
}
class NavigationMesh
{
public:
	NavigationMesh();
	~NavigationMesh();
	void GenTestMesh();
	void DrawNavMeshLines(class DebugLineDrawer * drawer);
	void PopulateNearLists();
	NavTriangle * FindTriangleFromWorldPos(glm::vec3 worldpos);
	ENavRequestStatus::Type CalculatePath(glm::vec3 Startpoint, glm::vec3 EndPos, NavigationPath* outputPath);
private:
	std::vector<NavTriangle*> Triangles;
};

