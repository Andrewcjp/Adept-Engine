#pragma once
#include "AI/ThirdParty/clipper.hpp"
namespace ClipperLib { class Clipper; }
class Scene;
struct NavPlane;
class NavMeshGenerator
{
public:
	NavMeshGenerator();
	~NavMeshGenerator();
	void Voxelise(Scene * TargetScene);
	NavPlane * GetPlane(float Z, std::vector<NavPlane*>& list);
	//Take the scene and generate a navigation mesh for it.
	void GenerateMesh(NavPlane* target);
	void RenderGrid();
private:
	ClipperLib::Clipper* ClipEngine = nullptr;
	const float SamplingDistance = 1.0f;
	ClipperLib::Paths output;
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
	float GridSpacing = 5.0f;
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
