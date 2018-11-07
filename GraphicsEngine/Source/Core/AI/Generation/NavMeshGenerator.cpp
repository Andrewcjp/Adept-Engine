
#include "NavMeshGenerator.h"
#include "AI/ThirdParty/clipper.hpp"
#include "Core/Platform/PlatformCore.h"
#include "Core/Platform/Logger.h"
#include "Physics/PhysicsEngine.h"
#include "Core/Utils/DebugDrawers.h"
#include "AI/ThirdParty/delaunator.hpp"
using namespace ClipperLib;
NavMeshGenerator::NavMeshGenerator()
{
	ClipEngine = new ClipperLib::Clipper();
}

NavMeshGenerator::~NavMeshGenerator()
{}

void NavMeshGenerator::Voxelise(Scene* TargetScene)
{
	//Find bounds of Scene

	//todo: load all meshes into the physics engine
	const int GridSize = 40;
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
				//DebugDrawers::DrawDebugLine(xypos, xypos + glm::vec3(0, -1 + 1 * hiot.Distance, 0), glm::vec3(0, 1, 0), false, 100);
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
	GenerateMesh(planes[0]);
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

void NavMeshGenerator::GenerateMesh(NavPlane* target)
{
	//test();
#if 0
	ClipperLib::Paths p(1);

	for (int i = 0; i < target->Points.size(); i++)
	{
		p[0] << ClipperLib::IntPoint(target->Points[i].x, target->Points[i].z);
	}
	ClipEngine->AddPath(p[0], ptClip, true);
	ClipEngine->AddPath(p[0], ptSubject, true);
	ClipEngine->Execute(ClipType::ctIntersection, output, pftNonZero, pftNonZero);
#else
	std::vector<double> Points;
	for (int i = 0; i < target->Points.size(); i++)
	{
		Points.push_back(target->Points[i].x);
		Points.push_back(0.0f);
		Points.push_back(target->Points[i].z);
	}
	delaunator::Delaunator d(Points);
	std::vector<Tri> triangles;
	for (std::size_t i = 0; i < d.triangles.size(); i += 3)
	{
		Tri newrti;
		newrti.points[0] = (glm::vec3(d.coords[2 * d.triangles[i]], 0, d.coords[2 * d.triangles[i] + 1]));
		newrti.points[1] = (glm::vec3(d.coords[2 * d.triangles[i + 1]], 0, d.coords[2 * d.triangles[i + 1] + 1]));
		newrti.points[2] = (glm::vec3(d.coords[2 * d.triangles[i + 2]], 0, d.coords[2 * d.triangles[i + 2] + 1]));
		if (true)
		{
			std::swap(newrti.points[0].x, newrti.points[0].z);
			std::swap(newrti.points[1].x, newrti.points[1].z);
			std::swap(newrti.points[2].x, newrti.points[2].z);
		}
		triangles.push_back(newrti);
	}
	for (int i = 0; i < triangles.size(); i++)
	{
		//if (i > 500)
		//{
		//	continue;
		//}
		const int sides = 3;
		for (int x = 0; x < sides; x++)
		{
			const int next = (x + 1) % sides;
			DebugDrawers::DrawDebugLine(triangles[i].points[x], triangles[i].points[next], glm::vec3(1), false, 1000);
		}

	}

#endif
}

void NavMeshGenerator::RenderGrid()
{
	return;
	for (int i = 0; i < output.size(); i++)
	{
		const int sides = output[i].size();
		for (int x = 0; x < sides; x++)
		{
			const int next = (x + 1) % sides;
			DebugDrawers::DrawDebugLine(glm::vec3(output[i][x].X, 0, output[i][x].Y), glm::vec3(output[i][next].X, 0, output[i][next].Y));
			//	drawer->AddLine(offset + Triangles[i]->Positons[x], offset + Triangles[i]->Positons[next], glm::vec3(1, 1, 1));
		}
		//for (int n = 0; n < Triangles[i]->NearTriangles.size(); n++)
		{
			//drawer->AddLine(offset + Triangles[i]->avgcentre, offset + Triangles[i]->avgcentre+glm::vec3(0,10,0), glm::vec3(1, 0, 0));
		}
	}

}
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
