#include "NavMeshGenerator.h"
#include "AI/Core/Navigation/DLTEPathfinder.h"
#include "Core/Platform/PlatformCore.h"
#include "Core/Utils/DebugDrawers.h"
#include "Core/Utils/VectorUtils.h"
#include "Physics/PhysicsEngine.h"
#include "ThirdParty/delaunator.hpp"
#include "Core/Performance/PerfManager.h"

NavMeshGenerator::NavMeshGenerator()
{}

NavMeshGenerator::~NavMeshGenerator()
{}

const float PlaneTolerance = 50.0f;
void NavMeshGenerator::Voxelise(Scene* TargetScene)
{
	Log::LogMessage("Started Nav Mesh Generation");

	Log::LogMessage("Started Height field creation");
	PerfManager::Get()->StartSingleActionTimer(VoxeliseTimer);
	const std::string HeightFieldTimer = "Navigation Mesh HeightField generation";
	const std::string FilterTimer = "Navigation Mesh Point Filtering";
	const std::string GenerateMeshTimer = "Navigation Mesh generation";
	const float worldMin = -10e10f;
	//Find bounds of Scene

	//todo: load all meshes into the physics engine
	const int GridSize = 80;
	HeightField* Field = new HeightField();
	Field->InitGrid(glm::vec3(0, 0, 0), GridSize + 20, GridSize);
	glm::vec3 pos = Field->GetPosition(GridSize / 2, GridSize / 2);
	//ensure(pos == glm::vec3(0, 0, 0));
	//Create height field from scene
	PerfManager::Get()->StartSingleActionTimer(HeightFieldTimer);
	int ValidPointCount = 0;
	const float distance = 50;
	for (int x = 0; x < GridSize; x++)
	{
		for (int y = 0; y < GridSize; y++)
		{
			glm::vec3 xypos = Field->GetPosition(x, y);
			xypos.y = startHeight;
			RayHit hiot;
			if (PhysicsEngine::Get()->RayCastScene(xypos, glm::vec3(0, -1, 0), distance, &hiot))
			{
				ValidPointCount++;
				Field->SetValue(x, y, -hiot.Distance);
				//DebugDrawers::DrawDebugLine(xypos, xypos - glm::vec3(0, -1 + 1 * hiot.Distance, 0), glm::vec3(0, 1, 0), false, 100);
			}
			else
			{
				Field->SetValue(x, y, worldMin);
			}
		}
	}
	
	Log::LogMessage("Height field contains " + std::to_string(ValidPointCount) + " Points");
	PerfManager::EndAndLogTimer(HeightFieldTimer);
	Log::LogMessage("Finished Height field creation");
	//to handle overhangs z partitioning will be used
	//Process Height Field to NavMesh
	//Perfect Quad Simplification 
	Log::LogMessage("Started Point Filtering");
	PerfManager::Get()->StartSingleActionTimer(FilterTimer);
	for (int i = 10; i > 1; i--)
	{
		GridFilter(GridSize, Field, worldMin, i);
	}
	PerfManager::EndAndLogTimer(FilterTimer);
	Log::LogMessage("Finished Point Filtering");
	//slopes will be handled with overall angle delta between point
	//plane object will take in points and create triangles which will then be simplified down to as few tri as possible
	PerfManager::Get()->StartSingleActionTimer(GenerateMeshTimer);
	for (int x = 0; x < GridSize; x++)
	{
		for (int y = 0; y < GridSize; y++)
		{
			//flood fill?
			//compute a plane of points near to a level?
			float PointHeight = Field->GetValue(x, y);
			if (PointHeight <= worldMin)
			{
				continue;
			}
			NavPlane* plane = GetPlane(PointHeight, planes);
			plane->Points.push_back(Field->GetPosition(x, y));
		}
	}

	for (int i = 0; i < planes.size(); i++)
	{
		if (planes[i]->Points.size() > 20)
		{
			Log::LogMessage("Generating mesh with " + std::to_string(planes[i]->Points.size()) + " Points");
			GenerateMesh(planes[i]);
		}
	}
	PerfManager::EndAndLogTimer(GenerateMeshTimer);
	PerfManager::EndAndLogTimer(VoxeliseTimer);
	Log::LogMessage("Finished Nav Mesh Generation");
}

void NavMeshGenerator::GridFilter(const int GridSize, HeightField* Field, const float worldMin, int QuadSize)
{
	const int GirdStep = 1;
	//const int QuadSize = 5;//GridSize / GirdStep;
	for (int x = 0; x < GridSize; x += GirdStep)
	{
		for (int y = 0; y < GridSize; y += GirdStep)
		{
			//check all are same height 
			//then set to reject height except 4 verts
			glm::ivec2 offset = glm::ivec2(x, y);
			float FirstHeight = Field->GetValue(x, y);
			if (ValidateQuad(QuadSize, FirstHeight, Field, offset))
			{
				if (FirstHeight <= worldMin)
				{
					continue;
				}
				for (int xx = 1; xx < QuadSize - 1; xx++)
				{
					for (int yy = 1; yy < QuadSize - 1; yy++)
					{
						RemovedQuadsPoints++;
						Field->SetValue(offset.x + xx, offset.y + yy, worldMin);
					}
				}
			}
		}
	}
	Log::LogMessage("Quad Stage of size " + std::to_string(QuadSize) + " Removed " + std::to_string(RemovedQuadsPoints) + " Points");
}

bool NavMeshGenerator::ValidateQuad(const int GirdStep, float FirstHeight, HeightField* Field, glm::ivec2 &offset)
{
	for (int xx = 0; xx < GirdStep; xx++)
	{
		for (int yy = 0; yy < GirdStep; yy++)
		{
			if (FirstHeight != Field->GetValue(offset.x + xx, offset.y + yy))
			{
				return false;
			}
		}
	}
	return true;
}

NavPlane * NavMeshGenerator::GetPlane(float Z)
{

	for (int i = 0; i < planes.size(); i++)
	{
		if (MathUtils::AlmostEqual(planes[i]->ZHeight, Z, PlaneTolerance))
		{
			return planes[i];
		}
	}
	return nullptr;
}

NavPlane* NavMeshGenerator::GetPlane(float Z, std::vector<NavPlane*>& list)
{
	NavPlane* plane = nullptr;
	for (int i = 0; i < list.size(); i++)
	{
		if (MathUtils::AlmostEqual(list[i]->ZHeight, Z, PlaneTolerance))
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
	Log::LogMessage("Triangle Generation start");
	std::vector<double> Points;
	int Mod = (int)target->Points.size() % 2;
	int DropAfter = (int)target->Points.size() - 1 - Mod;
	for (int i = 0; i < target->Points.size(); i++)
	{
		if (i >= DropAfter)
		{
			continue;
		}
		Points.push_back(target->Points[i].x);
		//Points.push_back(startHeight - target->Points[i].y);
		Points.push_back(target->Points[i].z);
	}

	//ensure(Points.size() % 3 == 0);
	delaunator::Delaunator d(Points);

	for (std::size_t i = 0; i < d.triangles.size(); i += 3)
	{
		Tri newrti;
#if 0
		newrti.points[0] = (glm::vec3(d.coords[2 * d.triangles[i]], startHeight + target->ZHeight, d.coords[2 * d.triangles[i] + 1]));
		newrti.points[1] = (glm::vec3(d.coords[2 * d.triangles[i + 1]], startHeight + target->ZHeight, d.coords[2 * d.triangles[i + 1] + 1]));
		newrti.points[2] = (glm::vec3(d.coords[2 * d.triangles[i + 2]], startHeight + target->ZHeight, d.coords[2 * d.triangles[i + 2] + 1]));
#else
		newrti.points[0] = (glm::vec3(d.coords[2 * d.triangles[i]], startHeight - target->Points[d.triangles[i]].y, d.coords[2 * d.triangles[i] + 1]));
		newrti.points[1] = (glm::vec3(d.coords[2 * d.triangles[i + 1]], startHeight - target->Points[d.triangles[i + 1]].y, d.coords[2 * d.triangles[i + 1] + 1]));
		newrti.points[2] = (glm::vec3(d.coords[2 * d.triangles[i + 2]], startHeight - target->Points[d.triangles[i + 2]].y, d.coords[2 * d.triangles[i + 2] + 1]));

#endif
		if (false)
		{
			std::swap(newrti.points[0].x, newrti.points[0].z);
			std::swap(newrti.points[1].x, newrti.points[1].z);
			std::swap(newrti.points[2].x, newrti.points[2].z);
		}
		target->Triangles.push_back(newrti);
	}
	//the delaunator Can generate triangles that stretch over invalid regions 
	//so: prune triangles that are invalid
#if 1
	TotalTriCount = (int)target->Triangles.size();
	for (int i = (int)target->Triangles.size() - 1; i >= 0; i--)
	{
		glm::vec3 avgpos;
		for (int n = 0; n < 3; n++)
		{
			avgpos += target->Triangles[i].points[n];
		}
		avgpos /= 3;
		RayHit hiot;
		avgpos.y = startHeight;
#if 0
		//DebugDrawers::DrawDebugLine(avgpos, avgpos + glm::vec3(0, target->ZHeight, 0), glm::vec3(0, 1, 0), false, 100);
		const bool CastHit = PhysicsEngine::Get()->RayCastScene(avgpos, glm::vec3(0, -1, 0), 50, &hiot);
		if (!CastHit || !MathUtils::AlmostEqual(hiot.Distance, -target->ZHeight, 5.0f))
		{
			target->Triangles.erase(target->Triangles.begin() + i);
			PrunedTris++;
		}
#else
		glm::vec3 dir = glm::cross(target->Triangles[i].points[1] - target->Triangles[i].points[0], target->Triangles[i].points[2] - target->Triangles[i].points[0]);
		glm::vec3 norm = glm::normalize(dir);

		float angle = glm::dot(norm, glm::vec3(0, 1, 0));
		if (angle < 0.7f)//remove tris that are angled wrong
		{
			target->Triangles.erase(target->Triangles.begin() + i);
			PrunedTris++;
		}
		//	Log::LogMessage("Pruning Points scene " + std::to_string(i) + "/" + std::to_string(target->Triangles.size()), Log::Severity::Progress);
#endif
	}
#endif
#if 0
	for (int i = 0; i < target->Triangles.size(); i++)
	{
		const int sides = 3;
		for (int x = 0; x < sides; x++)
		{
			const int next = (x + 1) % sides;
			//DebugDrawers::DrawDebugLine(target->Triangles[i].points[x], target->Triangles[i].points[next], -glm::vec3(target->ZHeight / startHeight), false, 1000);
		}
		}
#endif
	target->BuildNavPoints();
	target->RemoveDupeNavPoints();
	target->BuildMeshLinks();
	std::stringstream ss;
	ss << "Pruned " << PrunedTris << "/" << TotalTriCount;
	Log::LogMessage(ss.str());
	//target->RenderMesh(false);
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
	for (int zx = 0; zx < Width*Height; zx++)
	{
		GridData[zx] = -std::numeric_limits<float>::max();
	}
}

bool Contains(DLTENode* point, std::vector<DLTENode*> & points, NavPlane* p)
{
	for (int i = 0; i < points.size(); i++)
	{
		if (points[i]->GetPos(p) == point->GetPos(p))
		{
			return true;
		}
	}
	return false;
}

void NavPlane::RemoveDupeNavPoints()
{
	std::vector<DLTENode*> RemoveList;
	for (int x = 0; x < NavPoints.size(); x++)
	{
		for (int y = 0; y < NavPoints.size(); y++)
		{
			if (x != y)
			{
				if (NavPoints[x]->Point == NavPoints[y]->Point)
				{
					if (VectorUtils::Contains<DLTENode*>(RemoveList, NavPoints[x], [](DLTENode*a, DLTENode* b)
					{
						return a->Point == b->Point;
					}))
					{
						continue;
					}

					RemoveList.push_back(NavPoints[y]);
					if (NavPoints[y]->OwnerTri->Nodes[0] == NavPoints[y])
					{
						NavPoints[y]->OwnerTri->Nodes[0] = NavPoints[x];
					}
					else if (NavPoints[y]->OwnerTri->Nodes[1] == NavPoints[y])
					{
						NavPoints[y]->OwnerTri->Nodes[1] = NavPoints[x];
					}
					else if (NavPoints[y]->OwnerTri->Nodes[2] == NavPoints[y])
					{
						NavPoints[y]->OwnerTri->Nodes[2] = NavPoints[x];
					}
				}
			}
		}
	}

	for (int i = 0; i < RemoveList.size(); i++)
	{
		VectorUtils::Remove(NavPoints, RemoveList[i]);
	}
}

void Link(DLTENode* a, DLTENode*b)
{
	a->NearNodes.push_back(b);
	b->NearNodes.push_back(a);
}

void NavPlane::BuildNavPoints()
{
	for (int i = 0; i < Triangles.size(); i++)
	{
		DLTENode* n1 = new DLTENode(Triangles[i].points[0]);
		DLTENode* n2 = new DLTENode(Triangles[i].points[1]);
		DLTENode* n3 = new DLTENode(Triangles[i].points[2]);
		NavPoints.push_back(n1);
		NavPoints.push_back(n2);
		NavPoints.push_back(n3);
		n1->OwnerTri = &Triangles[i];
		n2->OwnerTri = &Triangles[i];
		n3->OwnerTri = &Triangles[i];
		Triangles[i].Nodes[0] = n1;
		Triangles[i].Nodes[1] = n2;
		Triangles[i].Nodes[2] = n3;
	}
}

void NavPlane::BuildMeshLinks()
{
	for (int i = 0; i < Triangles.size(); i++)
	{
		Link(Triangles[i].Nodes[0], Triangles[i].Nodes[1]);
		Link(Triangles[i].Nodes[1], Triangles[i].Nodes[2]);
		Link(Triangles[i].Nodes[2], Triangles[i].Nodes[0]);
	}

	for (int x = 0; x < NavPoints.size(); x++)
	{
		for (int y = 0; y < NavPoints.size(); y++)
		{
			if (NavPoints[x]->GetPos(this) == NavPoints[y]->GetPos(this) && x != y)
			{
				if (!Contains(NavPoints[x], NavPoints[x]->NearNodes, this))
				{
					Link(NavPoints[x], NavPoints[y]);
				}
			}
		}
	}
}

void NavPlane::RenderMesh(bool Near)
{
	if (Near)
	{
		for (int i = 0; i < NavPoints.size(); i++)
		{
			for (int x = 0; x < NavPoints[i]->NearNodes.size(); x++)
			{
				float value = (float)x + 1 / (float)NavPoints[i]->NearNodes.size();
				glm::vec3 dir = NavPoints[i]->GetPos(this) - NavPoints[i]->NearNodes[x]->GetPos(this);
				DebugDrawers::DrawDebugLine(NavPoints[i]->GetPos(this), NavPoints[i]->GetPos(this) + dir / 3, glm::vec3(value), false, 1000);
			}
		}
	}
	else
	{
		for (int i = 0; i < Triangles.size(); i++)
		{
			const int sides = 3;
			for (int x = 0; x < sides; x++)
			{
				const int next = (x + 1) % sides;
				DebugDrawers::DrawDebugLine(Triangles[i].points[x], Triangles[i].points[next], glm::vec3(1), false, 0.0f);
			}
		}
	}
}

bool NavPlane::ResolvePositionToNode(glm::vec3 pos, DLTENode ** node)
{
#if 0
	//Triangle phase first 
	Tri* triangle = FindTriangleFromWorldPos(pos);
	if (triangle == nullptr)
	{
		return false;
	}

	float CurrentPoint = FloatMAX;
	for (int i = 0; i < 3; i++)
	{
		const float newdist = glm::distance2(triangle->Nodes[i]->GetPos(this), pos);
		if (newdist < CurrentPoint)
		{
			CurrentPoint = newdist;
			*node = triangle->Nodes[i];
		}
			}
#else
	float CurrentPoint = MathUtils::FloatMAX;
	for (int i = 0; i < NavPoints.size(); i++)
	{
		const float newdist = glm::distance2(NavPoints[i]->GetPos(this), pos);
		if (newdist < CurrentPoint)
		{
			CurrentPoint = newdist;
			*node = NavPoints[i];
		}
	}
#endif
	//the check points
	return true;
		}

Tri* NavPlane::FindTriangleFromWorldPos(glm::vec3 worldpos)
{
	for (int i = 0; i < Triangles.size(); i++)
	{
		if (Triangles[i].IsPointInsideTri(worldpos))
		{
			return &Triangles[i];
		}
	}
	return nullptr;
}

void NavPlane::Reset()
{
	for (int i = 0; i < NavPoints.size(); i++)
	{
		NavPoints[i]->Reset();
	}
}

float NavPlane::GetHeight()
{
	return ZHeight;
}

bool PointInTriangle(glm::vec3 p, glm::vec3 p0, glm::vec3 p1, glm::vec3 p2)
{
	float s = p0.z * p2.x - p0.x * p2.z + (p2.z - p0.z) * p.x + (p0.x - p2.x) * p.z;
	float t = p0.x * p1.z - p0.z * p1.x + (p0.z - p1.z) * p.x + (p1.x - p0.x) * p.z;

	if ((s < 0) != (t < 0))
		return false;

	float A = -p1.z * p2.x + p0.z * (p2.x - p1.x) + p0.x * (p1.z - p2.z) + p1.x * p2.z;

	return A < 0 ?
		(s <= 0 && s + t >= A) :
		(s >= 0 && s + t <= A);
}

bool Tri::IsPointInsideTri(glm::vec3 point)
{
	return PointInTriangle(point, points[0], points[1], points[2]);
}
