#pragma once

class Scene;
class NavPlane;
struct HeightField;
struct DLTENode;
class NavMeshGenerator
{
public:
	NavMeshGenerator();
	~NavMeshGenerator();
	void Voxelise(Scene * TargetScene);
	bool ValidateQuad(const int GirdStep, float FirstHeight, HeightField* Field, glm::ivec2 &offset);
	NavPlane* GetPlane(float z);	
	//Take the scene and generate a navigation mesh for it.
	void GenerateMesh(NavPlane* target);
private:
	NavPlane * GetPlane(float Z, std::vector<NavPlane*>& list);
	const float SamplingDistance = 1.0f;
	int RemovedQuadsPoints = 0;
	int PrunedTris = 0;
	int TotalTriCount = 0;
	std::vector<NavPlane*> planes;
};
struct HeightField 
{
	void SetValue(int x, int y, float value);
	float GetValue(int x, int y);
	glm::vec3 GetPosition(int x, int y);
	void InitGrid(glm::vec3 pos,int x, int y);
private:
	float* GridData = nullptr;
	int Width, Height = 0;
	glm::vec3 RootPos = glm::vec3(0, 0, 0);
	glm::vec3 CentreOffset = glm::vec3(0, 0, 0);
	float GridSpacing = 2.5f;
};

struct Tri
{
	glm::vec3 points[3];
	DLTENode* Nodes[3];
	bool IsPointInsideTri(glm::vec3 point);
};

class NavPlane
{
public:
	void RemoveDupeNavPoints();
	void BuildNavPoints();
	void BuildMeshLinks();
	void RenderMesh(bool Near);	
	bool ResolvePositionToNode(glm::vec3 pos, DLTENode** node);
	Tri * FindTriangleFromWorldPos(glm::vec3 worldpos);
	void Reset();
	float GetHeight();
	std::vector<glm::vec3> Points;
	std::vector<Tri> Triangles;
	float ZHeight = 0.0f;
private:
	std::vector<DLTENode*> NavPoints;	

};
