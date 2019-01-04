#include "TDMeshShape.h"
#include "TDBox.h"
#include "TDBVH.h"
#include "TDCollisionHandlers.h"
#include "TDPhysics.h"
#include "TDPlane.h"
#include "TDSAT.h"
#include "TDSimConfig.h"
#include "TDSphere.h"
#include "Utils/MathUtils.h"

namespace TD
{
	TDMeshShape::TDMeshShape(TDMesh* mesh)
	{
		ShapeType = TDShapeType::eTRIANGLEMESH;
		Mesh = mesh;
	}

	TDMeshShape::~TDMeshShape()
	{
		SafeDelete(Mesh);
	}

	glm::vec3 TDMeshShape::GetBoundBoxHExtents()
	{
		if (Mesh != nullptr)
		{
			return(Mesh->Max - Mesh->Min) * 0.5f;
		}
		return glm::vec3(0);//todo: this!
	}

	TDAABB * TDMeshShape::GetAABB()
	{
		return Mesh->GetBVH()->Root->bounds;
	}

	TDTriangle::TDTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c)
	{
		Points[0] = a;
		Points[1] = b;
		Points[2] = c;
		posAVG = (a + b + c) / 3;
	}

	TDTriangle::TDTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 normal) :TDTriangle(a, b, c)
	{
		Normal = normal;
	}

	TD::TDPlane TDTriangle::MakeFromTriangle()
	{
		const glm::vec3 normal = glm::normalize(glm::cross(Points[1] - Points[0], Points[2] - Points[0]));
		TDPlane result = TDPlane(normal);
		result.PlaneDistance = glm::dot(normal, Points[0]);
		return result;
	}

	bool TDTriangle::PointInTriangle(const glm::vec3& p)
	{
		//Translate the point to the origin of the triangle
		glm::vec3 a = Points[0] - p;
		glm::vec3 b = Points[1] - p;
		glm::vec3 c = Points[2] - p;
		//P is effectively removed
		glm::vec3 normPointBC = glm::cross(b, c); // Normal of PBC (u)
		glm::vec3 normPointCA = glm::cross(c, a); // Normal of PCA (v)
		glm::vec3 normPointAB = glm::cross(a, b); // Normal of PAB (w)

		//Test if normals are facing 
		if (glm::dot(normPointBC, normPointCA) < 0.0f)
		{
			return false;
		}
		else if (glm::dot(normPointBC, normPointAB) < 0.0f)
		{
			return false;
		}

		// All normals facing the same way, return true
		return true;
	}

	glm::vec3 Edge::ClosestPoint(const glm::vec3 point)
	{
		glm::vec3 lVec = PointB - PointA; // Vector of Line 
		float t = glm::dot(point - PointA, lVec) / glm::dot(lVec, lVec);
		t = glm::clamp(t, 0.0f, 1.0f);
		// the projected position of t
		return PointA + lVec * t;
	}

	bool TDMeshShape::MeshSphere(TDSphere * s, ContactData* contactbuffer)
	{
		const bool IsTrigger = s->GetFlags().GetFlagValue(TDShapeFlags::ETrigger);
		std::vector<TriangleInterection> InterSections;
		if (Mesh->GetBVH()->TraverseForSphere(s, InterSections, IsTrigger ? 1 : 0))//Generates a list of intersections with this Sphere
		{
			for (int i = 0; i < InterSections.size(); i++)
			{
				const glm::vec3 normal = InterSections[i].Tri->Normal;
				contactbuffer->Contact(InterSections[i].Point, normal, InterSections[i].depth);
#if !BUILD_SHIPPING
				InterSections[i].Tri->DebugDraw(0.0f);
#endif
			}
		}
		return contactbuffer->Blocking;
	}

	bool TDMeshShape::MeshBox(TDBox* Box, ContactData* Contacts)
	{
		const bool IsTrigger = Box->GetFlags().GetFlagValue(TDShapeFlags::ETrigger);
		std::vector<TriangleInterection> InterSections;
		if (Mesh->GetBVH()->TraverseForBox(Box, InterSections, IsTrigger ? 1 : 0))//Generates a list of intersections with this Box
		{
			for (int i = 0; i < InterSections.size(); i++)
			{
				const glm::vec3 normal = InterSections[i].Tri->Normal;
				Contacts->Contact(InterSections[i].Point, normal, InterSections[i].depth);
#if !BUILD_SHIPPING
				InterSections[i].Tri->DebugDraw(0.0f);
#endif
			}
		}
		return Contacts->Blocking;
	}

	bool TDMeshShape::IntersectTriangle(RayCast* ray)
	{
		return Mesh->GetBVH()->TraverseForRay(ray, this);
	}

	//Just some simple maths:
	glm::vec3 TDTriangle::GetBarycentric(const glm::vec3& p)
	{
		const glm::vec3 ap = p - Points[0];
		const glm::vec3 bp = p - Points[1];
		const glm::vec3 cp = p - Points[2];

		const glm::vec3 ab = Points[1] - Points[0];
		const glm::vec3 ac = Points[2] - Points[0];
		const glm::vec3 bc = Points[2] - Points[1];
		const glm::vec3 cb = Points[1] - Points[2];
		const glm::vec3 ca = Points[0] - Points[2];

		glm::vec3 v = ab - MathUtils::VectorProject(ab, cb);
		const float a = 1.0f - (glm::dot(v, ap) / glm::dot(v, ab));

		v = bc - MathUtils::VectorProject(bc, ac);
		const float b = 1.0f - (glm::dot(v, bp) / glm::dot(v, bc));

		v = ca - MathUtils::VectorProject(ca, ab);
		const float c = 1.0f - (glm::dot(v, cp) / glm::dot(v, ca));
		return glm::vec3(a, b, c);
	}

	bool TDTriangle::Intersect(RayCast* ray, TDMeshShape* owner)
	{
		TDPlane plane = MakeFromTriangle();
		if (!TDIntersectionHandlers::IntersectPlane(&plane, ray))
		{
			return false;
		}
		const float RayDist = ray->HitData->Points[0].Distance;
		if (RayDist > ray->Distance)
		{
			ray->HitData->Reset();
			return false;
		}
		ray->HitData->Reset();
		glm::vec3 BaryCoords = GetBarycentric(ray->Origin + ray->Dir * RayDist);
		if ((BaryCoords.x >= 0.0f && BaryCoords.x <= 1.0f) &&
			(BaryCoords.y >= 0.0f && BaryCoords.y <= 1.0f) &&
			(BaryCoords.z >= 0.0f && BaryCoords.z <= 1.0f))
		{
			ray->HitData->BlockingHit = true;
			ray->HitData->AddContact(ray->Origin + ray->Dir * RayDist, plane.Normal, RayDist, owner);
			return true;
		}
		ray->HitData->BlockingHit = false;
		return false;
	}

	bool TDTriangle::TriangleSphere(TDSphere * s, glm::vec3 & out_Contact, float & depth)
	{
		//find the point closest to the sphere and then check if it inside it
		glm::vec3 closest = ClosestPoint(s->GetPos());
		out_Contact = closest;
		float magSq = glm::length2(closest - s->GetPos());
		depth = (magSq - (s->Radius * s->Radius));
		return magSq <= s->Radius * s->Radius;
	}

	glm::vec3 TDTriangle::ClosestPoint(const glm::vec3& p)
	{
		TDPlane plane = MakeFromTriangle();
		glm::vec3 closest = plane.ClosestPoint(p);

		// Closest point was inside triangle
		if (PointInTriangle(closest))
		{
			return closest;
		}
		//Make 3 edges
		glm::vec3 c1 = Edge(Points[0], Points[1]).ClosestPoint(closest); // Line AB
		glm::vec3 c2 = Edge(Points[1], Points[2]).ClosestPoint(closest); // Line BC
		glm::vec3 c3 = Edge(Points[2], Points[0]).ClosestPoint(closest); // Line CA

		float magSq1 = glm::length2(closest - c1);
		float magSq2 = glm::length2(closest - c2);
		float magSq3 = glm::length2(closest - c3);

		if (magSq1 < magSq2 && magSq1 < magSq3)
		{
			return c1;
		}
		else if (magSq2 < magSq1 && magSq2 < magSq3)
		{
			return c2;
		}

		return c3;
	}

	void TDTriangle::DebugDraw(float time)
	{
		if (!TDPhysics::GetCurrentSimConfig()->ShowContacts)
		{
			return;
		}
		for (int i = 0; i < 3; i++)
		{
			const int next = (i + 1) % 3;
			TDPhysics::DrawDebugLine(Points[i], Points[next], glm::vec3(1), time);
		}
	}

	TDMesh::TDMesh(const TDTriangleMeshDesc& desc)
	{
		//takes in data in most compatible format and turns it into the TD internal one
		//todo: data stride!
		for (int i = 0; i < desc.Indices.Count; i += 3)
		{
			glm::vec3 posa = *desc.Points.GetFromArray<glm::vec3>(*desc.Indices.GetFromArray<int>(i));
			glm::vec3 posb = *desc.Points.GetFromArray<glm::vec3>(*desc.Indices.GetFromArray<int>(i + 1));
			glm::vec3 posc = *desc.Points.GetFromArray<glm::vec3>(*desc.Indices.GetFromArray<int>(i + 2));

			glm::vec3 Normal = glm::vec3(0, 0, 0);
			if (desc.HasPerVertexNormals)
			{
				Normal += *desc.Normals.GetFromArray<glm::vec3>(*desc.Indices.GetFromArray<int>(i));
				Normal += *desc.Normals.GetFromArray<glm::vec3>(*desc.Indices.GetFromArray<int>(i));
				Normal += *desc.Normals.GetFromArray<glm::vec3>(*desc.Indices.GetFromArray<int>(i));
				Normal /= 3;
				glm::normalize(Normal);
			}
			Triangles.push_back(new TDTriangle(posa, posb, posc, Normal));
		}
		CookMesh();
	}

	TDMesh::~TDMesh()
	{
		SafeDelete(BVH);
	}

	void TDMesh::CookMesh()
	{
		//Compute the Bounds for this object
		Min = Triangles[0]->Points[0];
		Max = Triangles[0]->Points[0];
		for (int i = 0; i < Triangles.size(); i++)
		{
			for (int x = 0; x < 3; x++)
			{
				Min = glm::min(Min, Triangles[i]->Points[x]);
				Max = glm::max(Max, Triangles[i]->Points[x]);
			}
		}
		BVH = new TDBVH();
		BVH->BuildAccelerationStructure(this);
	}

	bool TDTriangle::TriangleAABB(const TDAABB* a)
	{
		// Edge normals of Tri
		glm::vec3 TriFace0 = Points[1] - Points[0];
		glm::vec3 TriFace1 = Points[2] - Points[1];
		glm::vec3 TriFace2 = Points[0] - Points[2];

		// AABB face normals
		glm::vec3 AABBFace0(1.0f, 0.0f, 0.0f);
		glm::vec3 AABBFace1(0.0f, 1.0f, 0.0f);
		glm::vec3 AABBFace2(0.0f, 0.0f, 1.0f);

		glm::vec3 Axises[13] = {
			AABBFace0,
			AABBFace1,
			AABBFace2,
			glm::cross(TriFace0, TriFace1),
			glm::cross(AABBFace0, TriFace0),
			glm::cross(AABBFace0, TriFace1),
			glm::cross(AABBFace0, TriFace2),
			glm::cross(AABBFace1, TriFace0),
			glm::cross(AABBFace1, TriFace1),
			glm::cross(AABBFace1, TriFace2),
			glm::cross(AABBFace2, TriFace0),
			glm::cross(AABBFace2, TriFace1),
			glm::cross(AABBFace2, TriFace2)
		};

		for (int i = 0; i < 13; ++i)
		{
			if (!TDSAT::OverlapOnAxis(a, this, Axises[i]))
			{
				return false; // Separating axis found
			}
		}

		return true; // Separating axis not found
	}

	bool TDTriangle::TriangleBox(TDBox * Box)
	{
		// Edge normals of Tri
		glm::vec3 TriFace0 = Points[1] - Points[0];
		glm::vec3 TriFace1 = Points[2] - Points[1];
		glm::vec3 TriFace2 = Points[0] - Points[2];

		// box face normals
		glm::mat3x3 orientation = glm::mat3(Box->GetTransfrom()->GetQuatRot());
		glm::vec3 BoxFace0 = glm::vec3(orientation[0][0], orientation[1][0], orientation[2][0]);
		glm::vec3 BoxFace1 = glm::vec3(orientation[0][1], orientation[1][1], orientation[2][1]);
		glm::vec3 BoxFace2 = glm::vec3(orientation[0][2], orientation[1][2], orientation[2][2]);

		glm::vec3 test[13] = {
			BoxFace0,
			BoxFace1,
			BoxFace2,
			glm::cross(TriFace0, TriFace1),
			glm::cross(BoxFace0, TriFace0),
			glm::cross(BoxFace0, TriFace1),
			glm::cross(BoxFace0, TriFace2),
			glm::cross(BoxFace1, TriFace0),
			glm::cross(BoxFace1, TriFace1),
			glm::cross(BoxFace1, TriFace2),
			glm::cross(BoxFace2, TriFace0),
			glm::cross(BoxFace2, TriFace1),
			glm::cross(BoxFace2, TriFace2)
		};

		for (int i = 0; i < 13; ++i)
		{
			if (!TDSAT::OverlapOnAxis(Box, this, test[i]))
			{
				return false; // Separating axis found
			}
		}
		return true; // Separating axis not found
	}

	glm::vec3 TDTriangle::GetPos()
	{
		return posAVG;
	}
}
