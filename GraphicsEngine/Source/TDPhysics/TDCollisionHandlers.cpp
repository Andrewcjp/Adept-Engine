#include "TDCollisionHandlers.h"
#include "Shapes/TDAABB.h"
#include "Shapes/TDBox.h"
#include "Shapes/TDMeshShape.h"
#include "Shapes/TDPlane.h"
#include "Shapes/TDSphere.h"
#include "Core/Utils/MathUtils.h"
#include "TDSAT.h"
namespace TD
{
	bool TD::TDCollisionHandlers::InvalidCollisonPair(CollisionHandlerArgs)
	{
		//assert(false);
		return false;
	}

	//convex
	bool TD::TDCollisionHandlers::CollideConvexConvex(CollisionHandlerArgs)
	{
		return false;
	}

	bool TD::TDCollisionHandlers::CollideConvexMesh(CollisionHandlerArgs)
	{
		return false;
	}

	//Spheres 
	bool TD::TDCollisionHandlers::CollideSphereSphere(CollisionHandlerArgs)
	{
		TDSphere* SphereA = TDShape::CastShape<TDSphere>(A);
		TDSphere* SphereB = TDShape::CastShape<TDSphere>(B);

		glm::vec3 CollisonNormal = SphereA->GetPos() - SphereB->GetPos();
		const float Distancesq = glm::length2(CollisonNormal);
		const float RaduisSum = SphereA->Radius + SphereB->Radius;
		const float inflatedSum = RaduisSum /*+ 0.1f*/;//mContactDistance
		if (Distancesq >= inflatedSum * inflatedSum)
		{
			return false;
		}
		float magn = glm::sqrt(Distancesq);
		//this would normally be a simple normalization of the CollisonNormal but the small number case has to handled here.
		if (magn <= 0.00001f)//the sphere are almost completely overlapped 
		{
			CollisonNormal = glm::vec3(10.0f, 10.0f, 10.0f);
		}
		else
		{
			CollisonNormal *= 1.0 / magn;
		}
		const glm::vec3 contactpoint = CollisonNormal * ((SphereA->Radius + magn - SphereB->Radius)*-0.5f) + SphereB->GetOwner()->GetTransfrom()->GetPos();
		contactbuffer->Contact(contactpoint, CollisonNormal, abs(magn - RaduisSum));
		return true;
	}

	bool TD::TDCollisionHandlers::CollideSpherePlane(CollisionHandlerArgs)
	{
		TDSphere* Sphere = TDShape::CastShape<TDSphere>(A);

		TDTransform* ATransform = A->GetTransfrom();
		TDTransform* BTransform = B->GetTransfrom();
		glm::vec3 SpherePos = glm::inverse(BTransform->GetModel())*glm::vec4(ATransform->GetPos(), 0);
		//BTransform->TransfromToLocalSpace()
		const float Seperation = SpherePos.y - Sphere->Radius;
		if (Seperation <= 0.0f)
		{
			const glm::vec3 normal = BTransform->GetUp();
			const glm::vec3 point = ATransform->GetPos() - normal * Sphere->Radius;
			contactbuffer->Contact(point, normal, abs(Seperation));
			return true;
		}
		return false;
	}

	bool TD::TDCollisionHandlers::CollideAABBAABB(TDAABB* A, TDAABB* B)
	{
		const glm::vec3 aMin = A->GetMin();
		const glm::vec3 aMax = A->GetMax();
		const glm::vec3 bMin = B->GetMin();
		const glm::vec3 bMax = B->GetMax();
		const bool result = (aMin.x <= bMax.x && aMax.x >= bMin.x) &&
			(aMin.y <= bMax.y && aMax.y >= bMin.y) &&
			(aMin.z <= bMax.z && aMax.z >= bMin.z);
		return result;
	}

	bool TDCollisionHandlers::AABBOBB(TDAABB * A, TDBox * B)
	{
		glm::mat3 o1 = glm::mat3(B->GetTransfrom()->GetQuatRot());

		glm::vec3 test[15] = 
		{
			glm::vec3(1, 0, 0), // AABB axis 1
			glm::vec3(0, 1, 0), // AABB axis 2
			glm::vec3(0, 0, 1), // AABB axis 3
			glm::vec3(o1[0][0], o1[1][0], o1[2][0]),
			glm::vec3(o1[0][1], o1[1][1], o1[2][1]),
			glm::vec3(o1[0][2], o1[1][2], o1[2][2]),
		};

		for (int i = 0; i < 3; ++i)
		{ 
			//Again, Fill out rest of axis
			test[6 + i * 3 + 0] = glm::cross(test[i], test[0]);
			test[6 + i * 3 + 1] = glm::cross(test[i], test[1]);
			test[6 + i * 3 + 2] = glm::cross(test[i], test[2]);
		}

		for (int i = 0; i < 15; ++i)
		{
			if (!TDSAT::OverlapOnAxis(A, B, test[i]))
			{
				return false; // Separating axis found
			}
		}
		return true; //No axises are separate
	}

	glm::vec3 ClosestPoint(const TDAABB* aabb, const glm::vec3& point)
	{
		glm::vec3 result = point;
		glm::vec3 min = aabb->GetMin();
		glm::vec3 max = aabb->GetMax();

		result.x = (result.x < min.x) ? min.x : result.x;
		result.y = (result.y < min.y) ? min.y : result.y;
		result.z = (result.z < min.z) ? min.z : result.z;

		result.x = (result.x > max.x) ? max.x : result.x;
		result.y = (result.y > max.y) ? max.y : result.y;
		result.z = (result.z > max.z) ? max.z : result.z;

		return result;
	}

	bool TD::TDCollisionHandlers::SphereAABB(TDSphere* sphere, const TDAABB* aabb)
	{
		glm::vec3 closestPoint = ClosestPoint(aabb, sphere->GetPos());
		float distSq = glm::length2(sphere->GetPos() - closestPoint);
		float radiusSq = sphere->Radius * sphere->Radius;
		return distSq <= radiusSq;//=
	}

	bool TD::TDCollisionHandlers::CollideSphereCapsule(CollisionHandlerArgs)
	{
		return false;
	}

	glm::vec3 ClosestPoint(TDBox* obb, const glm::vec3& point)
	{
		glm::vec3 result = obb->GetPos();
		glm::vec3 dir = point - obb->GetPos();
		for (int i = 0; i < 3; ++i)
		{
			glm::vec3 axis = obb->Rotation[i];
			float distance = glm::dot(dir, axis);

			if (distance > obb->HalfExtends[i])
			{
				distance = obb->HalfExtends[i];
			}
			if (distance < -obb->HalfExtends[i])
			{
				distance = -obb->HalfExtends[i];
			}
			result = result + (axis * distance);
		}
		return result;
	}
	bool TD::TDCollisionHandlers::CollideSphereBox(CollisionHandlerArgs)
	{
		//todo: check this
		TDSphere* sphere = TDShape::CastShape<TDSphere>(A);
		TDBox* box = TDShape::CastShape<TDBox>(B);
		glm::vec3 closestPoint = ClosestPoint(box, sphere->GetPos());
		float distSq = glm::length2(sphere->GetPos() - closestPoint);
		float radiusSq = sphere->Radius * sphere->Radius;
		const float sepeation = radiusSq - distSq;
		if (distSq < radiusSq)
		{
			contactbuffer->Contact(closestPoint, -(B->GetPos() - A->GetPos()), abs(sepeation));
			return true;
		}
		return false;
	}

	bool TD::TDCollisionHandlers::CollideSphereConvex(CollisionHandlerArgs)
	{
		return false;
	}

	bool TD::TDCollisionHandlers::CollideSphereMesh(CollisionHandlerArgs)
	{
		TDSphere* sphere = TDShape::CastShape<TDSphere>(A);
		TDMeshShape* mesh = TDShape::CastShape<TDMeshShape>(B);
		return mesh->MeshSphere(sphere, contactbuffer);
	}

	//Plane
	bool TD::TDCollisionHandlers::CollidePlaneCapsule(CollisionHandlerArgs)
	{
		return false;
	}

	bool TD::TDCollisionHandlers::CollidePlaneBox(CollisionHandlerArgs)
	{
		TDPlane* plane = TDShape::CastShape<TDPlane>(A);
		TDBox* box = TDShape::CastShape<TDBox>(B);

		glm::vec3 normal = plane->GetNormal();

		// Project the half extents of the AABB onto the plane normal
		float pLen = box->HalfExtends.x * fabsf(glm::dot(normal, box->Rotation[0])) +
			box->HalfExtends.y * fabsf(glm::dot(normal, box->Rotation[1])) +
			box->HalfExtends.z * fabsf(glm::dot(normal, box->Rotation[2]));
		// Find the distance from the center of the OBB to the plane
		float dist = glm::dot(plane->GetNormal(), box->GetPos()) /*- plane.distance*/;
		// Intersection occurs if the distance falls within the projected side
		if (fabsf(dist) <= pLen)
		{
			const float seperation = pLen - dist;
			glm::vec3 point = ClosestPoint(box, box->GetPos());
			contactbuffer->Contact(box->GetPos(), plane->GetNormal(), abs(seperation));
			return true;
		}
		return false;
	}

	bool TD::TDCollisionHandlers::CollidePlaneConvex(CollisionHandlerArgs)
	{
		return false;
	}

	bool TD::TDCollisionHandlers::CollidePlaneMesh(CollisionHandlerArgs)
	{
		return false;
	}

	//Capsule
	bool TD::TDCollisionHandlers::CollideCapsuleCapsule(CollisionHandlerArgs)
	{
		return false;
	}

	bool TD::TDCollisionHandlers::CollideCapsuleBox(CollisionHandlerArgs)
	{
		return false;
	}

	bool TD::TDCollisionHandlers::CollideCapsuleConvex(CollisionHandlerArgs)
	{
		return false;
	}

	bool TD::TDCollisionHandlers::CollideCapsuleMesh(CollisionHandlerArgs)
	{
		return false;
	}

	//Box
	bool TD::TDCollisionHandlers::CollideBoxBox(CollisionHandlerArgs)
	{
		TDBox* ABox = TDShape::CastShape< TDBox>(A);
		TDBox* BBox = TDShape::CastShape< TDBox>(B);
		glm::mat3 o1 = glm::mat3(ABox->GetTransfrom()->GetQuatRot());
		glm::mat3 o2 = glm::mat3(BBox->GetTransfrom()->GetQuatRot());

		glm::vec3 test[15] =
		{
			glm::vec3(o1[0][0], o1[1][0], o1[2][0]),
			glm::vec3(o1[0][1], o1[1][1], o1[2][1]),
			glm::vec3(o1[0][2], o1[1][2], o1[2][2]),
			glm::vec3(o2[0][0], o2[1][0], o2[2][0]),
			glm::vec3(o2[0][1], o2[1][1], o2[2][1]),
			glm::vec3(o2[0][2], o2[1][2], o2[2][2]),
		};

		for (int i = 0; i < 3; ++i)
		{ // Fill out rest of axis
			test[6 + i * 3 + 0] = glm::cross(test[i], test[0]);
			test[6 + i * 3 + 1] = glm::cross(test[i], test[1]);
			test[6 + i * 3 + 2] = glm::cross(test[i], test[2]);
		}
		int Minindex = 0;
		float minValue = 10000;
		for (int i = 0; i < 15; i++)
		{
			float v = 1000;
			if (!TDSAT::OverlapOnAxis(ABox, BBox, test[i], v))
			{
				return false; // Separating axis found
			}
			if (minValue > v && i < 6)
			{
				minValue = v;
				Minindex = i;
			}
		}
		const glm::vec3 Point = ABox->ClosestPoint(BBox->GetPos());
		glm::vec3 normal = -test[Minindex];
		const glm::vec3 NormalRayEnd = Point + (normal * 100);
		const glm::vec3 FurtherestExtent = ABox->ClosestPoint(NormalRayEnd);
		float depth = glm::length(FurtherestExtent - Point) / 32;
		//TDPhysics::DrawDebugLine(Point, Point + normal * 2, glm::vec3(1, 1, 1), 0.0f);
		contactbuffer->Contact(Point, normal, depth);//todo: depth is not right here
		return true;//No axis are separate so intersection is present
	}

	bool TD::TDCollisionHandlers::CollideBoxConvex(CollisionHandlerArgs)
	{
		return false;
	}

	bool TD::TDCollisionHandlers::CollideBoxMesh(CollisionHandlerArgs)
	{
		TDBox* Box = TDShape::CastShape<TDBox>(A);
		TDMeshShape* mesh = TDShape::CastShape<TDMeshShape>(B);
		return mesh->MeshBox(Box, contactbuffer);
	}

	bool TD::TDIntersectionHandlers::IntersectSphere(InterSectionArgs)
	{
		return false;
	}

	bool TD::TDIntersectionHandlers::IntersectPlane(InterSectionArgs)//TDShape* Shape,glm::vec3 Origin,glm::vec3 Dir,float distance
	{
		TDPlane* plane = TDShape::CastShape<TDPlane>(Shape);

		float nd = glm::dot(Ray->Dir, plane->Normal);
		float pn = glm::dot(Ray->Origin, plane->Normal);

		// nd must be negative, and not 0
		// if nd is positive, the ray and plane normals
		// point in the same direction. No intersection.
		if (nd >= 0.0f)
		{
			return false;
		}

		float t = (plane->PlaneDistance - pn) / nd;

		// t must be positive
		if (t >= 0.0f)
		{
			if (t <= Ray->Distance)
			{
				Ray->HitData->AddContact(Ray->Origin + Ray->Dir * t, plane->Normal, t);
				return true;
			}
		}

		return false;
	}

	bool TD::TDIntersectionHandlers::IntersectCapsule(InterSectionArgs)
	{
		return false;
	}

	bool TD::TDIntersectionHandlers::IntersectBox(InterSectionArgs)
	{
		return false;
	}

	bool TD::TDIntersectionHandlers::IntersectConvex(InterSectionArgs)
	{
		return false;
	}

	bool TD::TDIntersectionHandlers::IntersectMesh(InterSectionArgs)
	{
		TDMeshShape* mesh = TDShape::CastShape<TDMeshShape>(Shape);
		return mesh->IntersectTriangle(Ray);
	}

	bool TDIntersectionHandlers::IntersectAABB(InterSectionArgs)
	{
		TDAABB* aabb = TDShape::CastShape<TDAABB>(Shape);

		glm::vec3 min = aabb->GetMin();
		glm::vec3 max = aabb->GetMax();

		// Any component of direction could be 0!
		// Address this by using a small number, close to
		// 0 in case any of directions components are 0
		float t1 = (min.x - Ray->Origin.x) / Ray->Dir.x;
		float t2 = (max.x - Ray->Origin.x) / Ray->Dir.x;
		float t3 = (min.y - Ray->Origin.y) / Ray->Dir.y;
		float t4 = (max.y - Ray->Origin.y) / Ray->Dir.y;
		float t5 = (min.z - Ray->Origin.z) / Ray->Dir.z;
		float t6 = (max.z - Ray->Origin.z) / Ray->Dir.z;

		float tmin = fmaxf(fmaxf(fminf(t1, t2), fminf(t3, t4)), fminf(t5, t6));
		float tmax = fminf(fminf(fmaxf(t1, t2), fmaxf(t3, t4)), fmaxf(t5, t6));

		// if tmax < 0, ray is intersecting AABB
		// but entire AABB is being it's origin
		if (tmax < 0)
		{
			return false;
		}

		// if tmin > tmax, ray doesn't intersect AABB
		if (tmin > tmax)
		{
			return false;
		}

		float t_result = tmin;

		// If tmin is < 0, tmax is closer
		if (tmin < 0.0f)
		{
			t_result = tmax;
		}

		if (Ray->HitData)
		{
			glm::vec3 normals[] = {
				glm::vec3(-1, 0, 0),
				glm::vec3(1, 0, 0),
				glm::vec3(0, -1, 0),
				glm::vec3(0, 1, 0),
				glm::vec3(0, 0, -1),
				glm::vec3(0, 0, 1)
			};
			float t[] = { t1, t2, t3, t4, t5, t6 };

			for (int i = 0; i < 6; ++i)
			{
				if (MathUtils::AlmostEqual(t_result, t[i], FLT_EPSILON))
				{
					Ray->HitData->AddContact(Ray->Origin + Ray->Dir* t_result, normals[i], t_result);
					return true;
				}
			}
		}
		return true;
	}
};