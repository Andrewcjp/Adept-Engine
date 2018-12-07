#include "TDCollisionHandlers.h"
#include "Shapes/TDAABB.h"
#include "Shapes/TDBox.h"
#include "Shapes/TDBox.h"
#include "Shapes/TDMeshShape.h"
#include "Shapes/TDPlane.h"
#include "Shapes/TDSphere.h"
#include "TDShape.h"
#include "Core/Utils/MathUtils.h"
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

		glm::vec3 CollisonNormal = SphereA->GetOwner()->GetTransfrom()->GetPos() - SphereB->GetOwner()->GetTransfrom()->GetPos();
		const float Distancesq = glm::length2(CollisonNormal);
		const float RaduisSum = SphereA->Radius + SphereB->Radius;
		const float inflatedSum = RaduisSum + 0.0f;//mContactDistance
		if (Distancesq >= inflatedSum * inflatedSum)
		{
			return false;
		}
		float magn = glm::sqrt(Distancesq);
		//this would normally be a simple normalization of the CollisonNormal but the small number case has to handled here.
		if (magn <= 0.00001f)//the sphere are almost completely overlapped 
		{
			CollisonNormal = glm::vec3(1.0f, 0.0f, 0.0f);
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

		TDTransform* ATransform = A->GetOwner()->GetTransfrom();
		TDTransform* BTransform = B->GetOwner()->GetTransfrom();
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
		return (aMin.x <= bMax.x && aMax.x >= bMin.x) &&
			(aMin.y <= bMax.y && aMax.y >= bMin.y) &&
			(aMin.z <= bMax.z && aMax.z >= bMin.z);
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
		glm::vec3 result = obb->GetOwner()->GetTransfrom()->GetPos();
		glm::vec3 dir = point - obb->GetOwner()->GetTransfrom()->GetPos();
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
		glm::vec3 closestPoint = ClosestPoint(box, sphere->GetOwner()->GetTransfrom()->GetPos());
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
		return false;
	}

	bool TD::TDCollisionHandlers::CollideBoxConvex(CollisionHandlerArgs)
	{
		return false;
	}

	bool TD::TDCollisionHandlers::CollideBoxMesh(CollisionHandlerArgs)
	{
		return false;
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
				Ray->HitData->Normal = glm::vec3(plane->Normal);
				Ray->HitData->Point = Ray->Origin + Ray->Dir * t;
				Ray->HitData->Distance = t;
				Ray->HitData->BlockingHit = true;
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
		// but entire AABB is behing it's origin
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
			Ray->HitData->Distance = t_result;
			Ray->HitData->BlockingHit = true;
			Ray->HitData->Point = Ray->Origin + Ray->Dir* t_result;

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
					Ray->HitData->Normal = normals[i];
				}
			}
		}

		return true;
	}
};