#pragma once

class Scene;
struct NavPlane;
struct HeightField;
class NavMeshGenerator
{
public:
	NavMeshGenerator();
	~NavMeshGenerator();
	void Voxelise(Scene * TargetScene);

	bool ValidateQuad(const int GirdStep, float FirstHeight, HeightField* Field, glm::ivec2 &offset);

	NavPlane * GetPlane(float Z, std::vector<NavPlane*>& list);
	//Take the scene and generate a navigation mesh for it.
	void GenerateMesh(NavPlane* target);
private:
	const float SamplingDistance = 1.0f;
	int RemovedQuadsPoints = 0;
	int PrunedTris = 0;
	int TotalTriCount = 0;
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
};
struct NavPlane
{
	std::vector<glm::vec3> Points;
	std::vector<Tri> Triangles;
	float ZHeight = 0.0f;
};
