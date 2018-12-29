#pragma once
class Scene;
class NavPlane;
struct HeightField;
struct DLTENode;
///Handles Creation of NavPlanes that hold data about a given nav mesh section.
class NavMeshGenerator
{
public:
	NavMeshGenerator();
	~NavMeshGenerator();
	///Takes a Scene (editor or Play) and Fires rays into it to build a Nav mesh 
	///Needs Physics representations to be loaded and valid.
	void Voxelise(Scene * TargetScene);

	void GridFilter(const int GridSize, HeightField* Field, const float worldMin, int QuadSize);

	bool ValidateQuad(const int GirdStep, float FirstHeight, HeightField* Field, glm::ivec2 &offset);
	///\returns the plane at the requested z height. Nullptr if not found.
	NavPlane* GetPlane(float z);
	///Take the NavPlane and generate a triangle mesh for it.
	void GenerateMesh(NavPlane* target);
private:
	NavPlane * GetPlane(float Z, std::vector<NavPlane*>& list);
	std::vector<NavPlane*> planes;
	//options
	const float startHeight = 10.0f;
	const float SamplingDistance = 1.0f;
	//stats 
	int RemovedQuadsPoints = 0;
	int PrunedTris = 0;
	int TotalTriCount = 0;	
	const std::string VoxeliseTimer = "Navigation Mesh Voxelise";	
};
///Contains a list of distances representing the Scene.
struct HeightField
{
	void SetValue(int x, int y, float value);
	float GetValue(int x, int y);
	glm::vec3 GetPosition(int x, int y);
	void InitGrid(glm::vec3 pos, int x, int y);
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
///Contains the Navigation nodes and Triangles
///Also responsible for generating and cleaning the triangles from the height field
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
