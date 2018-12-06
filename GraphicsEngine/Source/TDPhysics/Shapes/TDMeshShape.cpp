
#include "TDMeshShape.h"
#include "TDPlane.h"
#include "TDSphere.h"
#include "TDPhysics.h"
#include "TDCollisionHandlers.h"

namespace TD
{
	TDMeshShape::TDMeshShape(TDMesh* mesh)
	{
		ShapeType = TDShapeType::eTRIANGLEMESH;
		Mesh = mesh;
	}


	TDMeshShape::~TDMeshShape()
	{}

	glm::vec3 TDMeshShape::GetBoundBoxHExtents()
	{
		return glm::vec3(10000);//todo: this!
	}

	TDTriangle::TDTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c)
	{
		Points[0] = a;
		Points[1] = b;
		Points[2] = c;
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
		return false;
	}

	bool TDMeshShape::IntersectTriangle(glm::vec3 Origin, glm::vec3 Dir, float distance, RaycastData* HitData)
	{
		for (int i = 0; i < Mesh->GetTriangles().size(); i++)
		{
			if (Mesh->GetTriangles()[i]->Intersect(Origin, Dir, distance, HitData))
			{
				return true;//todo: Multicast!
			}
		}
		return false;
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

	bool TDTriangle::Intersect(glm::vec3 Origin, glm::vec3 Dir, float distance, RaycastData* HitData)
	{
		TDPlane plane = MakeFromTriangle();
		if (!TDIntersectionHandlers::IntersectPlane(&plane, Origin, Dir, distance, HitData))
		{
			return false;
		}
		const float t = HitData->Distance;
		if (t > distance)
		{
			return false;
		}
		glm::vec3 BaryCoords = GetBarycentric(Origin + Dir * t);
		if ((BaryCoords.x >= 0.0f && BaryCoords.x <= 1.0f) &&
			(BaryCoords.y >= 0.0f && BaryCoords.y <= 1.0f) &&
			(BaryCoords.z >= 0.0f && BaryCoords.z <= 1.0f))
		{
			HitData->BlockingHit = true;
			HitData->Distance = t;
			HitData->Normal = plane.Normal;
			HitData->Point = Origin + Dir * t;
			return true;
		}
		HitData->BlockingHit = false;		
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

	void TDTriangle::DebugDraw()
	{
		for (int i = 0; i < 3; i++)
		{
			const int next = (i + 1) % 3;
			TDPhysics::DrawDebugLine(Points[i], Points[next], glm::vec3(1), 0.0f);
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
	}

	void TDMesh::CookMesh()
	{

	}
}