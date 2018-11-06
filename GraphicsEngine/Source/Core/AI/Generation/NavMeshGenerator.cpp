
#include "NavMeshGenerator.h"
#include "AI/ThirdParty/clipper.hpp"
#include "Core/Platform/PlatformCore.h"
#include "Core/Platform/Logger.h"
#include "Physics/PhysicsEngine.h"
using namespace ClipperLib;
NavMeshGenerator::NavMeshGenerator()
{}

NavMeshGenerator::~NavMeshGenerator()
{}

void NavMeshGenerator::Voxelise(Scene* TargetScene)
{
	//Find bounds of Scene

	//todo: load all meshes into the physics engine
	const int GridSize = 100;
	HeightField* Field = new HeightField();
	Field->InitGrid(glm::vec3(0, 0, 0), GridSize, GridSize);
	glm::vec3 pos = Field->GetPosition(GridSize / 2, GridSize / 2);
	//ensure(pos == glm::vec3(0, 0, 0));
	//Create height field from scene
	const float distance = 10;
	for (int x = 0; x < GridSize; x++)
	{
		for (int y = 0; y < GridSize; y++)
		{
			glm::vec3 xypos = Field->GetPosition(x, y);
			xypos.y = 0.0f;
			RayHit hiot;
			if (PhysicsEngine::Get()->RayCastScene(xypos, glm::vec3(0, -1, 0), distance, &hiot))
			{
				Field->SetValue(x, y, hiot.Distance);
			}
		}
	}
	//to handle overhangs z partitioning will be used
	//Process Height Field to NavMesh

	//slopes will be handled with overall angle delta between point
	//plane object will take in points and create triangles which will then be simplified down to as few tri as possible
	const float worldMin = -10e-10f;
	std::vector<NavPlane*> planes;
	for (int x = 0; x < GridSize; x++)
	{
		for (int y = 0; y < GridSize; y++)
		{
			//flood fill?
			//compute a plane of points near to a level?
			float PointHeight = Field->GetValue(x, y);
			if (PointHeight < worldMin)
			{
				continue;
			}
			NavPlane* plane = GetPlane(PointHeight, planes);
			plane->Points.push_back(Field->GetPosition(x, y));
		}
	}

}
bool approximatelyEqual(float a, float b, float epsilon)
{
	return fabs(a - b) <= ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}
NavPlane* NavMeshGenerator::GetPlane(float Z, std::vector<NavPlane*>& list)
{
	const float PlaneTolerance = 0.5f;
	NavPlane* plane = nullptr;
	for (int i = 0; i < list.size(); i++)
	{
		if (approximatelyEqual(list[i]->ZHeight, Z, PlaneTolerance))
		{
			return list[i];
		}
	}
	plane = new NavPlane();
	plane->ZHeight = Z;
	list.push_back(plane);
	return plane;
}

void NavMeshGenerator::GenerateMesh(Scene * TargetScene)
{
	//ClipEngine->Execute(ClipType::ctIntersection, );
}
//Paths subj(2), clip(1), solution;
//
////define outer blue 'subject' polygon
//subj[0] <<
//IntPoint(180, 200) << IntPoint(260, 200) <<
//IntPoint(260, 150) << IntPoint(180, 150);
//
////define subject's inner triangular 'hole' (with reverse orientation)
//subj[1] <<
//IntPoint(215, 160) << IntPoint(230, 190) << IntPoint(200, 190);
//
////define orange 'clipping' polygon
//clip[0] <<
//IntPoint(190, 210) << IntPoint(240, 210) <<
//IntPoint(240, 130) << IntPoint(190, 130);
//
////draw input polygons with user-defined routine ... 
//DrawPolygons(subj, 0x160000FF, 0x600000FF); //blue
//DrawPolygons(clip, 0x20FFFF00, 0x30FF0000); //orange
//
////perform intersection ...
//Clipper c;
//c.AddPaths(subj, ptSubject, true);
//c.AddPaths(clip, ptClip, true);
//c.Execute(ctIntersection, solution, pftNonZero, pftNonZero);
//
////draw solution with user-defined routine ... 
//DrawPolygons(solution, 0x3000FF00, 0xFF006600); //solution shaded green

void HeightField::SetValue(int x, int y, float value)
{
	GridData[x * Width + y] = value;
}

float HeightField::GetValue(int x, int y)
{
	return GridData[x * Width + y];
}

glm::vec3 HeightField::GetPosition(int x, int y)
{
	const glm::vec3 rawpos = glm::vec3(RootPos.x + x * GridSpacing, GetValue(x, y), RootPos.z + y * GridSpacing);
	return CentreOffset - rawpos;
}

void HeightField::InitGrid(glm::vec3 Pos, int x, int y)
{
	Width = x;
	Height = y;
	RootPos = Pos;
	CentreOffset = glm::vec3((x / 2)*GridSpacing, 0, (y / 2)*GridSpacing);
	GridData = new float[Width*Height];
	for (int x = 0; x < Width*Height; x++)
	{
		GridData[x] = -std::numeric_limits<float>::max();
	}
}
