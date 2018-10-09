#include "Stdafx.h"
#include "NavigationMesh.h"
#include "Core/Assets/MeshLoader.h"
#include "Rendering/Core/DebugLineDrawer.h"

NavigationMesh::NavigationMesh()
{}


NavigationMesh::~NavigationMesh()
{}

void NavigationMesh::GenTestMesh()
{
	std::vector<OGLVertex> vertices;
	std::vector<int> indices;
	MeshLoader::FMeshLoadingSettings t;
	t.Scale = glm::vec3(5);
	MeshLoader::LoadMeshFromFile("C:\\Users\\AANdr\\Dropbox\\Engine\\Engine\\Repo\\GraphicsEngine\\Content\\models\\NavPlaneTest_L.obj", t, vertices, indices);

	for (int i = 0; i < vertices.size(); i += 3)
	{
		NavTriangle* a = new NavTriangle();
		a->Positons[0] = vertices[indices[i]].m_position;
		a->Positons[1] = vertices[indices[i + 1]].m_position;
		a->Positons[2] = vertices[indices[i + 2]].m_position;

		for (int x = 0; x < 3; x++)
		{
			a->avgcentre += a->Positons[x];
		}
		a->avgcentre /= 3;
		Triangles.push_back(a);
	}
	PopulateNearLists();
	NavigationPath data;
	CalculatePath(glm::vec3(10, 0, 0), glm::vec3(0, 0, 0), &data);
}

void NavigationMesh::DrawNavMeshLines(DebugLineDrawer* drawer)
{
	glm::vec3 offset = glm::vec3(0, 3, 0);
	for (int i = 0; i < Triangles.size(); i++)
	{
		for (int x = 0; x < 3; x++)
		{
			const int next = (x + 1) % 3;
			drawer->AddLine(offset + Triangles[i]->Positons[x], offset + Triangles[i]->Positons[next], glm::vec3(1, 1, 1));
		}
		for (int n = 0; n < Triangles[i]->NearTriangles.size(); n++)
		{
			drawer->AddLine(offset + Triangles[i]->avgcentre, offset + Triangles[i]->NearTriangles[n]->avgcentre, glm::vec3(1, 0, 0));
		}
	}
}

struct Edge
{
	Edge(glm::vec3 f, glm::vec3 s)
	{
		first = f;
		second = s;
	}
	glm::vec3 first;
	glm::vec3 second;
	NavTriangle* tri;
	bool operator==(Edge lhs)
	{
		return (lhs.first == first) && (lhs.second == second) || (lhs.second == first) && (lhs.first == second);
	}
};
int findInV(std::vector<Edge>& v, Edge value)
{
	for (int i = 0; i < v.size(); i++)
	{
		if (value == v[i])
		{
			return i;
		}
	}
	return -1;
}
void NavigationMesh::PopulateNearLists()
{
	std::vector<Edge> SharedEdges;
	for (int i = 0; i < Triangles.size(); i++)
	{
		for (int x = 0; x < 3; x++)
		{
			const int next = (x + 1) % 3;
			Edge e = Edge(Triangles[i]->Positons[x], Triangles[i]->Positons[next]);
			e.tri = Triangles[i];
			int index = findInV(SharedEdges, e);
			if (index != -1)
			{
				SharedEdges[index].tri->NearTriangles.push_back(Triangles[i]);
				Triangles[i]->NearTriangles.push_back(SharedEdges[index].tri);
			}
			else
			{
				SharedEdges.push_back(e);
			}
		}
	}
}
NavTriangle* NavigationMesh::FindTriangleFromWorldPos(glm::vec3 worldpos)
{
	for (int i = 0; i < Triangles.size(); i++)
	{
		if (Triangles[i]->IsPointInsideTri(worldpos))
		{
			return Triangles[i];
		}
	}
	return nullptr;
}
ENavRequestStatus::Type NavigationMesh::CalculatePath(glm::vec3 Startpoint, glm::vec3 EndPos, NavigationPath* outputPath)
{
	*outputPath = NavigationPath();
	NavTriangle* StartTri = FindTriangleFromWorldPos(Startpoint);
	if (StartTri == nullptr)
	{
		return ENavRequestStatus::FailedPointOffNavMesh;
	}
	NavTriangle* EndTri = FindTriangleFromWorldPos(EndPos);
	if (EndTri == nullptr)
	{
		return ENavRequestStatus::FailedPointOffNavMesh;
	}



	return ENavRequestStatus::Complete;
}

float side(glm::vec2 v1, glm::vec2  v2, glm::vec2 point)
{
	return (v2.y - v1.y)*(point.x - v1.x) + (-v2.x + v1.x)*(point.y - v1.y);
}

bool pointInTriangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 point)
{
	bool checkSide1 = side(v1.xz, v2.xz, point.xy) >= 0;
	bool checkSide2 = side(v2.xz, v3.xz, point.xy) >= 0;
	bool checkSide3 = side(v3.xz, v1.xz, point.xy) >= 0;
	return checkSide1 && checkSide2 && checkSide3;
}
bool NavTriangle::IsPointInsideTri(glm::vec3 point)
{
	return pointInTriangle(Positons[0], Positons[1], Positons[2], point);
}
