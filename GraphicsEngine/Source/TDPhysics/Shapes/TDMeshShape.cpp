
#include "TDMeshShape.h"
#include "TDPlane.h"
#include "TDSphere.h"
#include "TDPhysics.h"
#include "TDCollisionHandlers.h"
#include "TDAABB.h"
#include "TDBVH.h"
#include "TDShape.h"
#include "TDTransform.h"
#include "TDBox.h"
#include "TDSAT.h"
#include "TDSimConfig.h"

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
		return glm::vec3(10);//todo: this!
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
		// Move the triangle so that the point is  
		// now at the origin of the triangle
		glm::vec3 a = Points[0] - p;
		glm::vec3 b = Points[1] - p;
		glm::vec3 c = Points[2] - p;

		// The point should be moved too, so they are both
		// relative, but because we don't use p in the
		// equation anymore, we don't need it!
		// p -= p; // This would just equal the zero vector!

		glm::vec3 normPBC = glm::cross(b, c); // Normal of PBC (u)
		glm::vec3 normPCA = glm::cross(c, a); // Normal of PCA (v)
		glm::vec3 normPAB = glm::cross(a, b); // Normal of PAB (w)

		// Test to see if the normals are facing 
		// the same direction, return false if not
		if (glm::dot(normPBC, normPCA) < 0.0f)
		{
			return false;
		}
		else if (glm::dot(normPBC, normPAB) < 0.0f)
		{
			return false;
		}

		// All normals facing the same way, return true
		return true;
	}

	glm::vec3 Edge::ClosestPoint(const Edge& line, const glm::vec3& point)
	{
		glm::vec3 lVec = line.pointb - line.pointa; // Line Vector
		// Project "point" onto the "Line Vector", computing:
		// closest(t) = start + t * (end - start)
		// T is how far along the line the projected point is
		float t = glm::dot(point - line.pointa, lVec) / glm::dot(lVec, lVec);
		// Clamp t to the 0 to 1 range
		t = fmaxf(t, 0.0f);
		t = fminf(t, 1.0f);
		// Return projected position of t
		return line.pointa + lVec * t;
	}

	bool TDMeshShape::MeshSphere(TDSphere * s, ContactData* contactbuffer)
	{
#if 0
		//normal is triangles normal!
		for (int i = 0; i < Mesh->GetTriangles().size(); i++)
		{
			float Depth = 0.0f;
			glm::vec3 ContactPoint = glm::vec3();
			if (Mesh->GetTriangles()[i]->TriangleSphere(s, ContactPoint, Depth))
			{
				const glm::vec3 normal = Mesh->GetTriangles()[i]->Normal;
				contactbuffer->Contact(ContactPoint, normal, Depth);
				DebugEnsure(Depth > -1.0f);
				DebugEnsure(Depth < 0.0f);
#if !BUILD_SHIPPING
				Mesh->GetTriangles()[i]->DebugDraw();
#endif
			}

		}
#else
		const bool IsTrigger = s->GetFlags().GetFlagValue(TDShapeFlags::ETrigger);
		std::vector<TriangleInterection> InterSections;
		if (Mesh->GetBVH()->TraverseForSphere(s, InterSections, IsTrigger ? 1 : 0))
		{
			for (int i = 0; i < InterSections.size(); i++)
			{
				const glm::vec3 normal = InterSections[i].Tri->Normal;
				contactbuffer->Contact(InterSections[i].Point, normal, InterSections[i].depth);
				//				DebugEnsure(InterSections[i].depth > -1.0f);
				//				DebugEnsure(InterSections[i].depth < 0.0f);
#if !BUILD_SHIPPING
				InterSections[i].Tri->DebugDraw(0.0f);
#endif
			}
		}

#endif
		return contactbuffer->Blocking;
	}
	bool TDMeshShape::MeshBox(TDBox* Box, ContactData* Contacts)
	{
		const bool IsTrigger = Box->GetFlags().GetFlagValue(TDShapeFlags::ETrigger);
		std::vector<TriangleInterection> InterSections;
		if (Mesh->GetBVH()->TraverseForBox(Box, InterSections, IsTrigger ? 1 : 0))
		{
			for (int i = 0; i < InterSections.size(); i++)
			{
				const glm::vec3 normal = InterSections[i].Tri->Normal;
				Contacts->Contact(InterSections[i].Point, normal, InterSections[i].depth);
				//				DebugEnsure(InterSections[i].depth > -1.0f);
				//				DebugEnsure(InterSections[i].depth < 0.0f);
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

	glm::vec3 Project(const glm::vec3& length, const glm::vec3& direction)
	{
		float dot = glm::dot(length, direction);
		float magSq = glm::length2(direction);
		return direction * (dot / magSq);
	}
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

		glm::vec3 v = ab - Project(ab, cb);
		const float a = 1.0f - (glm::dot(v, ap) / glm::dot(v, ab));

		v = bc - Project(bc, ac);
		const float b = 1.0f - (glm::dot(v, bp) / glm::dot(v, bc));

		v = ca - Project(ca, ab);
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
		const float t = ray->HitData->Points[0].Distance;
		if (t > ray->Distance)
		{
			ray->HitData->Reset();
			return false;
		}
		ray->HitData->Reset();
		glm::vec3 BaryCoords = GetBarycentric(ray->Origin + ray->Dir * t);
		if ((BaryCoords.x >= 0.0f && BaryCoords.x <= 1.0f) &&
			(BaryCoords.y >= 0.0f && BaryCoords.y <= 1.0f) &&
			(BaryCoords.z >= 0.0f && BaryCoords.z <= 1.0f))
		{
			ray->HitData->BlockingHit = true;
			ray->HitData->AddContact(ray->Origin + ray->Dir * t, plane.Normal, t, owner);
			return true;
		}
		ray->HitData->BlockingHit = false;
		return false;
	}

	bool TDTriangle::TriangleSphere(TDSphere * s, glm::vec3 & out_Contact, float & depth)
	{
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

		glm::vec3 c1 = Edge::ClosestPoint(Edge(Points[0], Points[1]), closest); // Line AB
		glm::vec3 c2 = Edge::ClosestPoint(Edge(Points[1], Points[2]), closest); // Line BC
		glm::vec3 c3 = Edge::ClosestPoint(Edge(Points[2], Points[0]), closest); // Line CA

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
	TDAABB* TDMesh::FromMinMax(const glm::vec3& min, const glm::vec3& max)
	{
		TDAABB* r = new TDAABB();
		r->HalfExtends = (max - min) * 0.5f;
		r->Position = (min + max) * 0.5f;
		return r;
	}
	void TDMesh::CookMesh()
	{
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

	Interval GetInterval(const TDTriangle* triangle, const glm::vec3& axis)
	{
		Interval result;

		result.min = glm::dot(axis, triangle->Points[0]);
		result.max = result.min;
		for (int i = 1; i < 3; ++i)
		{
			float value = glm::dot(axis, triangle->Points[i]);
			result.min = fminf(result.min, value);
			result.max = fmaxf(result.max, value);
		}

		return result;
	}

	Interval GetInterval(TDBox* obb, const glm::vec3& axis)
	{
		glm::vec3 vertex[8];

		glm::vec3 C = obb->GetPos();	// OBB Center
		glm::vec3 E = obb->HalfExtends;		// OBB Extents
		//const float* o = obb->GetTransfrom;
		//glm::vec3 A[] = {			// OBB Axis
		//	glm::vec3(o[0], o[1], o[2]),
		//	glm::vec3(o[3], o[4], o[5]),
		//	glm::vec3(o[6], o[7], o[8]),
		//};
		glm::mat3x3 A = glm::mat3(obb->GetTransfrom()->GetQuatRot());
		glm::vec3 u0 = glm::vec3(A[0][0], A[1][0], A[2][0]);
		glm::vec3 u1 = glm::vec3(A[0][1], A[1][1], A[2][1]);//todo: wrong way round?
		glm::vec3 u2 = glm::vec3(A[0][2], A[1][2], A[2][2]);

		vertex[0] = C + A[0] * E[0] + A[1] * E[1] + A[2] * E[2];
		vertex[1] = C - A[0] * E[0] + A[1] * E[1] + A[2] * E[2];
		vertex[2] = C + A[0] * E[0] - A[1] * E[1] + A[2] * E[2];
		vertex[3] = C + A[0] * E[0] + A[1] * E[1] - A[2] * E[2];
		vertex[4] = C - A[0] * E[0] - A[1] * E[1] - A[2] * E[2];
		vertex[5] = C + A[0] * E[0] - A[1] * E[1] - A[2] * E[2];
		vertex[6] = C - A[0] * E[0] + A[1] * E[1] - A[2] * E[2];
		vertex[7] = C - A[0] * E[0] - A[1] * E[1] + A[2] * E[2];

		Interval result;
		result.min = result.max = glm::dot(axis, vertex[0]);

		for (int i = 1; i < 8; ++i)
		{
			float projection = glm::dot(axis, vertex[i]);
			result.min = (projection < result.min) ? projection : result.min;
			result.max = (projection > result.max) ? projection : result.max;
		}

		return result;
	}

	bool OverlapOnAxis(const TDAABB* aabb, const TDTriangle* triangle, const glm::vec3& axis)
	{
		Interval a = TDSAT::GetInterval(aabb, axis);
		Interval b = GetInterval(triangle, axis);
		return ((b.min <= a.max) && (a.min <= b.max));
	}

	bool OverlapOnAxis(TDBox* obb, const TDTriangle* triangle, const glm::vec3& axis)
	{
		Interval a = GetInterval(obb, axis);
		Interval b = GetInterval(triangle, axis);
		return ((b.min <= a.max) && (a.min <= b.max));
	}

	bool TDTriangle::TriangleAABB(const TDAABB* a)
	{
		// Compute the edge vectors of the triangle  (ABC)
		glm::vec3 f0 = Points[1] - Points[0];
		glm::vec3 f1 = Points[2] - Points[1];
		glm::vec3 f2 = Points[0] - Points[2];

		// Compute the face normals of the AABB
		glm::vec3 u0(1.0f, 0.0f, 0.0f);
		glm::vec3 u1(0.0f, 1.0f, 0.0f);
		glm::vec3 u2(0.0f, 0.0f, 1.0f);

		glm::vec3 test[13] = {
			// 3 Normals of AABB
			u0, // AABB Axis 1
			u1, // AABB Axis 2
			u2, // AABB Axis 3
			// 1 Normal of the Triangle
			glm::cross(f0, f1),
			// 9 Axis, cross products of all edges
			glm::cross(u0, f0),
			glm::cross(u0, f1),
			glm::cross(u0, f2),
			glm::cross(u1, f0),
			glm::cross(u1, f1),
			glm::cross(u1, f2),
			glm::cross(u2, f0),
			glm::cross(u2, f1),
			glm::cross(u2, f2)
		};

		for (int i = 0; i < 13; ++i)
		{
			if (!OverlapOnAxis(a, this, test[i]))
			{
				return false; // Separating axis found
			}
		}

		return true; // Separating axis not found
	}

	bool TDTriangle::TriangleBox(TDBox * Box)
	{
		// Compute the edge vectors of the triangle  (ABC)
		glm::vec3 f0 = Points[1] - Points[0];
		glm::vec3 f1 = Points[2] - Points[1];
		glm::vec3 f2 = Points[0] - Points[2];

		// Compute the face normals of the Box / OBB
		glm::mat3x3 orientation = glm::mat3(Box->GetTransfrom()->GetQuatRot());
		glm::vec3 u0 = glm::vec3(orientation[0][0], orientation[1][0], orientation[2][0]);
		glm::vec3 u1 = glm::vec3(orientation[0][1], orientation[1][1], orientation[2][1]);
		glm::vec3 u2 = glm::vec3(orientation[0][2], orientation[1][2], orientation[2][2]);
		glm::vec3 test[13] = {
			// 3 Normals of AABB
			u0, // AABB Axis 1
			u1, // AABB Axis 2
			u2, // AABB Axis 3
			// 1 Normal of the Triangle
			glm::cross(f0, f1),
			// 9 Axis, cross products of all edges
			glm::cross(u0, f0),
			glm::cross(u0, f1),
			glm::cross(u0, f2),
			glm::cross(u1, f0),
			glm::cross(u1, f1),
			glm::cross(u1, f2),
			glm::cross(u2, f0),
			glm::cross(u2, f1),
			glm::cross(u2, f2)
		};

		for (int i = 0; i < 13; ++i)
		{
			if (!OverlapOnAxis(Box, this, test[i]))
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
