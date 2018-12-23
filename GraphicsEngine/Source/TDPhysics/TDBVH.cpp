#include "TDBVH.h"
#include "Shapes/TDAABB.h"
#include "Shapes/TDMeshShape.h"
#include "TDCollisionHandlers.h"
#include <queue>
#include "TDPhysics.h"
#include "TDSimConfig.h"
#include "TDShape.h"
#include "Shapes/TDBox.h"
namespace TD
{

	TDBVH::TDBVH()
	{}

	TDBVH::~TDBVH()
	{
		FreeBVHNode(Root);
		SafeDelete(Root);
	}

	void TDBVH::BuildAccelerationStructure(TDMesh * Mesh)
	{
		TargetMesh = Mesh;
		Root = new BVHNode();
		Root->bounds = TDMesh::FromMinMax(Mesh->Min, Mesh->Max);
		Root->children.clear();
		Root->numTriangles = (int)Mesh->GetTriangles().size();
		for (int i = 0; i < Mesh->GetTriangles().size(); i++)
		{
			Root->TrianglesIndexs.push_back(i);
		}
		MaxDepth = TDPhysics::GetCurrentSimConfig()->MaxBVHDepth;
		Targetcount = TDPhysics::GetCurrentSimConfig()->TargetTrianglesPerBVHNode;
		SplitBVH(Root, Mesh, 0);
	}

	void TDBVH::SplitBVH(BVHNode * node, TDMesh * model, int depth)
	{
		depth++;
		if (depth >= MaxDepth)
		{
			return;
		}
		if (node->numTriangles < Targetcount)
		{
			return;
		}
		if (node->children.size() == 0)
		{
			if (node->numTriangles > 0)
			{
				//split the node 
				node->children.resize(8);
				glm::vec3 c = node->bounds->Position;
				glm::vec3 e = node->bounds->HalfExtends *0.5f;

				node->children[0].bounds = new TDAABB(c + glm::vec3(-e.x, +e.y, -e.z), e);
				node->children[1].bounds = new TDAABB(c + glm::vec3(+e.x, +e.y, -e.z), e);
				node->children[2].bounds = new TDAABB(c + glm::vec3(-e.x, +e.y, +e.z), e);
				node->children[3].bounds = new TDAABB(c + glm::vec3(+e.x, +e.y, +e.z), e);
				node->children[4].bounds = new TDAABB(c + glm::vec3(-e.x, -e.y, -e.z), e);
				node->children[5].bounds = new TDAABB(c + glm::vec3(+e.x, -e.y, -e.z), e);
				node->children[6].bounds = new TDAABB(c + glm::vec3(-e.x, -e.y, +e.z), e);
				node->children[7].bounds = new TDAABB(c + glm::vec3(+e.x, -e.y, +e.z), e);
			}
		}

		if (node->children.size() != 0 && node->numTriangles > 0)//just split as there are still Triangles in this node
		{
			for (int i = 0; i < 8; i++)
			{
				node->children[i].numTriangles = 0;
				for (int j = 0; j < node->numTriangles; j++)
				{
					TDTriangle* tri = model->GetTriangles()[node->TrianglesIndexs[j]];
					if (tri->TriangleAABB(node->children[i].bounds))
					{
						/*tri->DebugDraw(100.0f);*/
						node->children[i].numTriangles++;
					}
				}
				if (node->children[i].numTriangles == 0)
				{
					continue;
				}
				node->children[i].TrianglesIndexs.resize(node->children[i].numTriangles);
				int index = 0; // Add the triangles in the new child array
				for (int j = 0; j < node->numTriangles; ++j)
				{
					TDTriangle* t = model->GetTriangles()[node->TrianglesIndexs[j]];
					if (t->TriangleAABB(node->children[i].bounds))
					{
						node->children[i].TrianglesIndexs[index++] = node->TrianglesIndexs[j];
					}
				}
			}
			node->numTriangles = 0;
			node->TrianglesIndexs.clear();

			// Recurse
			for (int i = 0; i < 8; ++i)
			{
				SplitBVH(&node->children[i], model, depth);
			}
		}
	}

	void TDBVH::FreeBVHNode(BVHNode* node)
	{
		if (node->children.size() != 0)
		{
			for (int i = 0; i < 8; ++i)
			{
				FreeBVHNode(&node->children[i]);
			}
			node->children.clear();
		}

		if (node->numTriangles != 0 || node->TrianglesIndexs.size() != 0)
		{
			node->TrianglesIndexs.clear();
			node->numTriangles = 0;
		}
	}

	bool TDBVH::TraverseForSphere(TDSphere * A, std::vector<TriangleInterection>& contacts, int MaxContactCount)
	{
		std::queue<BVHNode*> toProcess;
		toProcess.emplace(Root);
		bool RetValue = false;
		// Recursively walk the BVH tree
		while (!toProcess.empty())
		{
			BVHNode* iterator = toProcess.front();
			toProcess.pop();
			if (iterator->numTriangles >= 0)
			{
				// Iterate trough all triangles of the node
				for (int i = 0; i < iterator->numTriangles; ++i)
				{
					// Triangle indices in BVHNode index the mesh
					TriangleInterection t;
					if (TargetMesh->GetTriangles()[iterator->TrianglesIndexs[i]]->TriangleSphere(A, t.Point, t.depth))
					{
						t.Tri = TargetMesh->GetTriangles()[iterator->TrianglesIndexs[i]];
						contacts.push_back(t);
						//iterator->bounds->DebugRender(glm::vec3(0, 1, 0));
						RetValue = true;
						if (MaxContactCount != 0)
						{
							if (contacts.size() >= MaxContactCount)
							{
								return RetValue;
							}
						}
					}
				}
			}
			//	iterator->bounds->DebugRender(glm::vec3(1, 0, 0));
			if (iterator->children.size())
			{
				for (int i = 0; i < 8; i++)
				{
					// Only push children whos bounds intersect the test geometry
					if (TDCollisionHandlers::SphereAABB(A, iterator->children[i].bounds))
					{
						toProcess.emplace(&iterator->children[i]);
					}
				}
			}
		}
		return RetValue;
	}

	bool TDBVH::TraverseForBox(TDBox * A, std::vector<TriangleInterection>& contacts, int MaxContactCount)
	{
#if 0
		bool RetValue = false;
		for (int i = 0; i < TargetMesh->GetTriangles().size(); i++)
		{
			TriangleInterection t;
			if (TargetMesh->GetTriangles()[i]->TriangleBox(A))
			{
				t.Tri = TargetMesh->GetTriangles()[i];
				const glm::vec3 ContactPoint = A->ClosestPoint(t.Tri->GetPos());
				const glm::vec3 NormalRayEnd = ContactPoint - (t.Tri->Normal * 100);
				const glm::vec3 FurtherestExtent = A->ClosestPoint(NormalRayEnd);
				t.depth = glm::length(FurtherestExtent - ContactPoint);
				t.Point = ContactPoint;
				contacts.push_back(t);
				RetValue = true;
			}
		}
#else
		std::queue<BVHNode*> toProcess;
		toProcess.emplace(Root);
		bool RetValue = false;
		// Recursively walk the BVH tree
		while (!toProcess.empty())
		{
			BVHNode* iterator = toProcess.front();
			toProcess.pop();
			if (iterator->numTriangles >= 0)
			{
				// Iterate trough all triangles of the node
				for (int i = 0; i < iterator->numTriangles; ++i)
				{
					// Triangle indices in BVHNode index the mesh

					if (TargetMesh->GetTriangles()[iterator->TrianglesIndexs[i]]->TriangleBox(A))
					{
						TriangleInterection t;
						t.Tri = TargetMesh->GetTriangles()[iterator->TrianglesIndexs[i]];
						const glm::vec3 ContactPoint = A->ClosestPoint(t.Tri->GetPos());
						const glm::vec3 NormalRayEnd = ContactPoint - (t.Tri->Normal * 100);
						const glm::vec3 FurtherestExtent = A->ClosestPoint(NormalRayEnd);
						t.depth = glm::length(FurtherestExtent - ContactPoint);
						t.Point = ContactPoint;
						contacts.push_back(t);
						//iterator->bounds->DebugRender(glm::vec3(0, 1, 0));
						RetValue = true;
						if (MaxContactCount != 0)
						{
							if (contacts.size() >= MaxContactCount)
							{
								return RetValue;
							}
						}
					}
				}
			}
			//	iterator->bounds->DebugRender(glm::vec3(1, 0, 0));
			if (iterator->children.size())
			{
				for (int i = 0; i < 8; i++)
				{
					// Only push children whos bounds intersect the test geometry
					if (TDCollisionHandlers::AABBOBB(iterator->children[i].bounds, A))
					{
						toProcess.emplace(&iterator->children[i]);
					}
				}
			}
		}
#endif
		return RetValue;
	}

	void TDBVH::Render()
	{
		std::queue<BVHNode*> toProcess;
		toProcess.push(Root);

		// Recursively walk the BVH tree
		while (!toProcess.empty())
		{
			BVHNode* iterator = toProcess.front();
			toProcess.pop();
			if (iterator->numTriangles > 0)
			{
				iterator->bounds->DebugRender(glm::vec3(0, 0, 1));
			}
			else
			{
				//iterator->bounds->DebugRender(glm::vec3(1, 0, 0));
			}

			for (int i = 0; i < iterator->children.size(); i++)
			{
				toProcess.emplace(&iterator->children[i]);
			}
		}
	}

	bool TDBVH::TraverseForRay(RayCast * ray, TDMeshShape* shape)
	{
		std::queue<BVHNode*> toProcess;
		toProcess.emplace(Root);
		bool RetValue = false;
		// Recursively walk the BVH tree
		while (!toProcess.empty())
		{
			BVHNode* iterator = toProcess.front();
			toProcess.pop();
			if (iterator->numTriangles >= 0)
			{
				// Iterate trough all triangles of the node
				for (int i = 0; i < iterator->numTriangles; ++i)
				{
					// Triangle indices in BVHNode index the mesh
					if (TargetMesh->GetTriangles()[iterator->TrianglesIndexs[i]]->Intersect(ray, shape))
					{
						return true;
						RetValue = true;
					}
				}
			}
			if (iterator->children.size())
			{
				for (int i = 0; i < 8; i++)
				{
					// Only push children whos bounds intersect the test geometry
					if (TDIntersectionHandlers::IntersectAABB(iterator->children[i].bounds, ray))
					{
						ray->HitData->Reset();
						toProcess.emplace(&iterator->children[i]);
					}
				}
			}
		}
		return RetValue;
	}
}