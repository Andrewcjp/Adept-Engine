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

	//Spheres 
	bool TD::TDCollisionHandlers::CollideSphereSphere(CollisionHandlerArgs)
	{
		TDSphere* SphereA = TDShape::CastShape<TDSphere>(A);
		TDSphere* SphereB = TDShape::CastShape<TDSphere>(B);

		glm::vec3 CollisonNormal = SphereA->GetPos() - SphereB->GetPos();
		const float Distancesq = glm::length2(CollisonNormal);
		const float RaduisSum = SphereA->Radius + SphereB->Radius;
		const float inflatedSum = RaduisSum + 0.01f;//inflate slightly
		if (Distancesq >= inflatedSum * inflatedSum)
		{
			return false;
		}
		float magn = glm::sqrt(Distancesq);
		//this would normally be a simple normalization of the CollisonNormal but the small number case has to handled here.
		if (magn <= 0.00001f)//the sphere are almost completely overlapped 
		{
			CollisonNormal = glm::vec3(1.0f, 1.0f, 1.0f);
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
		glm::vec3 SpherePos = glm::inverse(BTransform->GetModel())*glm::vec4(ATransform->GetPos(), 0);//get the Spheres Position in the planes local space

		const float Seperation = SpherePos.y - Sphere->Radius;
		if (Seperation <= 0.0f)
		{
			const glm::vec3 normal = BTransform->GetUp();//Planes are always up in the world of TD sim
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
		glm::mat3 RotationMat = glm::mat3(B->GetTransfrom()->GetQuatRot());

		glm::vec3 TestAxis[15] =
		{
			glm::vec3(1, 0, 0), // AABB axis 1
			glm::vec3(0, 1, 0), // AABB axis 2
			glm::vec3(0, 0, 1), // AABB axis 3
			glm::vec3(RotationMat[0][0], RotationMat[1][0], RotationMat[2][0]),
			glm::vec3(RotationMat[0][1], RotationMat[1][1], RotationMat[2][1]),
			glm::vec3(RotationMat[0][2], RotationMat[1][2], RotationMat[2][2]),
		};

		for (int i = 0; i < 3; ++i)
		{
			//Again, Fill out rest of axis
			TestAxis[6 + i * 3 + 0] = glm::cross(TestAxis[i], TestAxis[0]);
			TestAxis[6 + i * 3 + 1] = glm::cross(TestAxis[i], TestAxis[1]);
			TestAxis[6 + i * 3 + 2] = glm::cross(TestAxis[i], TestAxis[2]);
		}

		for (int i = 0; i < 15; ++i)
		{
			if (!TDSAT::OverlapOnAxis(A, B, TestAxis[i]))
			{
				return false; // Separating axis found
			}
		}
		return true; //All axis are separate
	}

	bool TD::TDCollisionHandlers::SphereAABB(TDSphere* sphere, const TDAABB* aabb)
	{
		glm::vec3 closestPoint = aabb->ClosestPoint(sphere->GetPos());
		float distSq = glm::length2(sphere->GetPos() - closestPoint);
		float radiusSq = sphere->Radius * sphere->Radius;
		return distSq <= radiusSq;
	}

	bool TD::TDCollisionHandlers::CollideSphereBox(CollisionHandlerArgs)
	{
		TDSphere* sphere = TDShape::CastShape<TDSphere>(A);
		TDBox* box = TDShape::CastShape<TDBox>(B);
		glm::vec3 closestPoint = box->ClosestPoint(sphere->GetPos());
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

	bool TD::TDCollisionHandlers::CollideSphereMesh(CollisionHandlerArgs)
	{
		TDSphere* sphere = TDShape::CastShape<TDSphere>(A);
		TDMeshShape* mesh = TDShape::CastShape<TDMeshShape>(B);
		return mesh->MeshSphere(sphere, contactbuffer);
	}

	bool TD::TDCollisionHandlers::CollidePlaneBox(CollisionHandlerArgs)
	{
		TDPlane* plane = TDShape::CastShape<TDPlane>(A);
		TDBox* box = TDShape::CastShape<TDBox>(B);

		const glm::vec3 normal = plane->GetNormal();

		// Project the half extents of the AABB onto the plane normal
		float pLen = box->HalfExtends.x * glm::abs(glm::dot(normal, box->Rotation[0])) +
			box->HalfExtends.y * glm::abs(glm::dot(normal, box->Rotation[1])) +
			box->HalfExtends.z * glm::abs(glm::dot(normal, box->Rotation[2]));
		// Find the distance from the center of the OBB to the plane
		float dist = glm::dot(plane->GetNormal(), box->GetPos());
		// Intersection occurs if the distance falls within the projected side
		if (glm::abs(dist) <= pLen)
		{
			const float seperation = pLen - dist;
			const glm::vec3 point = box->ClosestPoint(box->GetPos());//todo: this is suspect
			contactbuffer->Contact(box->GetPos(), plane->GetNormal(), abs(seperation));
			return true;
		}
		return false;
	}

	//Box
	bool TD::TDCollisionHandlers::CollideBoxBox(CollisionHandlerArgs)/**/
	{
		TDBox* ABox = TDShape::CastShape< TDBox>(A);
		TDBox* BBox = TDShape::CastShape< TDBox>(B);
		glm::mat3 BoxARot = glm::mat3(ABox->GetTransfrom()->GetQuatRot());
		glm::mat3 BoxBRot = glm::mat3(BBox->GetTransfrom()->GetQuatRot());

		glm::vec3 Axis[15] =
		{
			glm::vec3(BoxARot[0][0], BoxARot[1][0], BoxARot[2][0]),
			glm::vec3(BoxARot[0][1], BoxARot[1][1], BoxARot[2][1]),
			glm::vec3(BoxARot[0][2], BoxARot[1][2], BoxARot[2][2]),
			glm::vec3(BoxBRot[0][0], BoxBRot[1][0], BoxBRot[2][0]),
			glm::vec3(BoxBRot[0][1], BoxBRot[1][1], BoxBRot[2][1]),
			glm::vec3(BoxBRot[0][2], BoxBRot[1][2], BoxBRot[2][2]),
		};

		for (int i = 0; i < 3; ++i)
		{ // Fill out rest of axis
			Axis[6 + i * 3 + 0] = glm::cross(Axis[i], Axis[0]);
			Axis[6 + i * 3 + 1] = glm::cross(Axis[i], Axis[1]);
			Axis[6 + i * 3 + 2] = glm::cross(Axis[i], Axis[2]);
		}
		int Minindex = 0;
		float minValue = 10000;
		for (int i = 0; i < 15; i++)
		{
			float v = 1000;
			if (!TDSAT::OverlapOnAxis(ABox, BBox, Axis[i], v))
			{
				return false; // Separating axis found
			}
			if (minValue > v && i < 6)
			{
				minValue = v;//find the smallest intersecting axis
				Minindex = i;
			}
		}
		const glm::vec3 Point = ABox->ClosestPoint(BBox->GetPos());
		glm::vec3 normal = -Axis[Minindex];
		const glm::vec3 NormalRayEnd = Point + (normal * 100);
		const glm::vec3 FurtherestExtent = BBox->ClosestPoint(NormalRayEnd);
		float depth = glm::length(FurtherestExtent - Point);
#if 0
		for (int i = 0; i < 15; i++)
		{
			bool Flip = false;
			float newdepth = PenetrationDepth(ABox, BBox, test[i], &Flip);
			if (newdepth <= 0.0f)
			{
				depth = newdepth;
				break;
			}
			else if (newdepth < depth)
			{
				if (Flip)
				{
					test[i] = test[i] * -1.0f;
				}
				depth = newdepth;
				normal = test[i];
			}
		}
#endif
		contactbuffer->Contact(Point, normal, depth);//todo: depth is suspect
		return true;//No axis are separate so intersection is present
	}

	bool TD::TDCollisionHandlers::CollideBoxMesh(CollisionHandlerArgs)
	{
		TDBox* Box = TDShape::CastShape<TDBox>(A);
		TDMeshShape* mesh = TDShape::CastShape<TDMeshShape>(B);
		return mesh->MeshBox(Box, contactbuffer);
	}

	bool TD::TDIntersectionHandlers::IntersectSphere(InterSectionArgs)
	{
		TDSphere* sphere = TDShape::CastShape<TDSphere>(Shape);

		glm::vec3 DirectionToRay = sphere->GetPos() - Ray->Origin;
		float RaduisSq = sphere->Radius * sphere->Radius;

		float eSq = glm::length2(DirectionToRay);
		float Alpha = glm::dot(DirectionToRay, Ray->Dir);//Dir is already normalized
		float bSq = eSq - (Alpha * Alpha);
		float f = glm::sqrt(glm::abs((RaduisSq)-bSq));

		// Assume normal intersection!
		float distance = Alpha - f;

		// No collision has happened
		if (RaduisSq - (eSq - Alpha * Alpha) < 0.0f)
		{
			return false;
		}
		// Ray starts inside the sphere
		else if (eSq < RaduisSq)
		{
			// Just reverse direction
			distance = Alpha + f;
		}
		const glm::vec3 Point = Ray->Origin + Ray->Dir* distance;
		Ray->HitData->AddContact(Point, Point - sphere->GetPos(), distance, Shape);
		return true;
	}

	bool TD::TDIntersectionHandlers::IntersectPlane(InterSectionArgs)//TDShape* Shape,Ray
	{
		TDPlane* plane = TDShape::CastShape<TDPlane>(Shape);

		float DirectionNormal = glm::dot(Ray->Dir, plane->Normal);
		float OriginNormal = glm::dot(Ray->Origin, plane->Normal);

		// nd must be negative, and not 0
		// if nd is positive, the ray and plane normals
		// point in the same direction. No intersection.
		if (DirectionNormal >= 0.0f)
		{
			return false;
		}

		float t = (plane->PlaneDistance - OriginNormal) / DirectionNormal;

		// t must be positive
		if (t >= 0.0f)
		{
			if (t <= Ray->Distance)
			{
				Ray->HitData->AddContact(Ray->Origin + Ray->Dir * t, plane->Normal, t, Shape);
				return true;
			}
		}

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
		//It is possible for some of these denominators top be 0.0f so Min with a Small number
		const float MinValue = 0.0000001f;
		float t1 = (min.x - Ray->Origin.x) / glm::min(Ray->Dir.x, MinValue);
		float t2 = (max.x - Ray->Origin.x) / glm::min(Ray->Dir.x, MinValue);
		float t3 = (min.y - Ray->Origin.y) / glm::min(Ray->Dir.y, MinValue);
		float t4 = (max.y - Ray->Origin.y) / glm::min(Ray->Dir.y, MinValue);
		float t5 = (min.z - Ray->Origin.z) / glm::min(Ray->Dir.z, MinValue);
		float t6 = (max.z - Ray->Origin.z) / glm::min(Ray->Dir.z, MinValue);

		float tmin = fmaxf(fmaxf(fminf(t1, t2), fminf(t3, t4)), fminf(t5, t6));
		float tmax = fminf(fminf(fmaxf(t1, t2), fmaxf(t3, t4)), fmaxf(t5, t6));

		if (tmax < 0.0f)
		{
			return false;
		}
		//No possible intersect
		if (tmin > tmax)
		{
			return false;
		}

		float Distance = tmin;

		// If tmin is < 0, tmax is closer
		if (tmin < 0.0f)
		{
			Distance = tmax;
		}

		if (Ray->HitData != nullptr)
		{
			glm::vec3 normals[] = //all possible normals
			{
				glm::vec3(-1, 0, 0),
				glm::vec3(1, 0, 0),
				glm::vec3(0, -1, 0),
				glm::vec3(0, 1, 0),
				glm::vec3(0, 0, -1),
				glm::vec3(0, 0, 1)
			};
			float t[] = { t1, t2, t3, t4, t5, t6 };//Al the computed mins and maxes
			const int BOX_SIDES = 6;
			for (int i = 0; i < BOX_SIDES; i++)
			{
				if (MathUtils::AlmostEqual(Distance, t[i], FLT_EPSILON))
				{
					if (!aabb->IsPartOfAcceleration)//Don't add a Contact if this is a Broadphase or BVH AABB
					{
						Ray->HitData->AddContact(Ray->Origin + Ray->Dir* Distance, normals[i], Distance, Shape);
					}
					return true;
				}
			}
		}
		return true;
	}

	//Currently Unused Functions (TODO)
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
	//Plane
	bool TD::TDCollisionHandlers::CollidePlaneCapsule(CollisionHandlerArgs)
	{
		return false;
	}

	bool TD::TDCollisionHandlers::CollideSphereConvex(CollisionHandlerArgs)
	{
		return false;
	}

	bool TD::TDCollisionHandlers::CollideSphereCapsule(CollisionHandlerArgs)
	{
		return false;
	}

	bool TD::TDCollisionHandlers::CollideBoxConvex(CollisionHandlerArgs)
	{
		return false;
	}

	bool TD::TDCollisionHandlers::InvalidCollisonPair(CollisionHandlerArgs)
	{
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
};