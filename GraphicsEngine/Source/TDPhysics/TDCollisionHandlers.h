#pragma once

#include "TDShape.h"
#include "TDTypes.h"

namespace TD
{
#define CollisionHandlerArgs TDShape* A, TDShape* B, ContactData* contactbuffer
	class TDAABB;
	struct ContactData;
	class TDCollisionHandlers
	{
	public:
		//Sphere Collisions
		static bool CollideSphereSphere(CollisionHandlerArgs);
		static bool CollideSpherePlane(CollisionHandlerArgs);
		static bool CollideSphereCapsule(CollisionHandlerArgs);
		static bool CollideSphereBox(CollisionHandlerArgs);
		static bool CollideSphereConvex(CollisionHandlerArgs);
		static bool CollideSphereMesh(CollisionHandlerArgs);
		//Plane Collisions
		static bool CollidePlaneCapsule(CollisionHandlerArgs);
		static bool CollidePlaneBox(CollisionHandlerArgs);
		static bool CollidePlaneConvex(CollisionHandlerArgs);
		static bool CollidePlaneMesh(CollisionHandlerArgs);
		//Capsule Collisions
		static bool CollideCapsuleCapsule(CollisionHandlerArgs);
		static bool CollideCapsuleBox(CollisionHandlerArgs);
		static bool CollideCapsuleConvex(CollisionHandlerArgs);
		static bool CollideCapsuleMesh(CollisionHandlerArgs);
		//Box Collisions
		static bool CollideBoxBox(CollisionHandlerArgs);
		static bool CollideBoxConvex(CollisionHandlerArgs);
		static bool CollideBoxMesh(CollisionHandlerArgs);
		//Convex Collisions
		static bool CollideConvexConvex(CollisionHandlerArgs);
		static bool CollideConvexMesh(CollisionHandlerArgs);
		
		//Invalid Pair
		static bool InvalidCollisonPair(CollisionHandlerArgs);
		static bool CollideAABBAABB(TDAABB * A, TDAABB * b);
		static bool SphereAABB(TDSphere * sphere, const TDAABB * aabb);

	};
#define InterSectionArgs TDShape* Shape,RayCast * Ray
	struct TDIntersectionHandlers
	{
		static bool IntersectSphere(InterSectionArgs);
		static bool IntersectPlane(InterSectionArgs);
		static bool IntersectCapsule(InterSectionArgs);
		static bool IntersectBox(InterSectionArgs);
		static bool IntersectConvex(InterSectionArgs);
		static bool IntersectMesh(InterSectionArgs);
		static bool IntersectAABB(InterSectionArgs);
	};

	typedef bool(*IntersectionMethod)(InterSectionArgs);
	static IntersectionMethod IntersectionMethodTable[TDShapeType::eLimit] =
	{
		TDIntersectionHandlers::IntersectSphere,
		TDIntersectionHandlers::IntersectPlane,
		TDIntersectionHandlers::IntersectCapsule,
		TDIntersectionHandlers::IntersectBox,
		TDIntersectionHandlers::IntersectConvex,
		TDIntersectionHandlers::IntersectMesh
	};
	typedef bool(*ContactMethod)(CollisionHandlerArgs);
	/**
	*\Brief Collision Handlers for every possible collision
	*/
	static ContactMethod ContactMethodTable[][TDShapeType::eLimit] =
	{
		//EShapeType::eSPHERE
		{
			TDCollisionHandlers::CollideSphereSphere,
			TDCollisionHandlers::CollideSpherePlane,
			TDCollisionHandlers::CollideSphereCapsule,
			TDCollisionHandlers::CollideSphereBox,
			TDCollisionHandlers::CollideSphereConvex,
			TDCollisionHandlers::CollideSphereMesh,
		},
		//EShapeType::Plane
		{
			0,
			TDCollisionHandlers::InvalidCollisonPair,//Planes can't collide with other planes
			TDCollisionHandlers::CollidePlaneCapsule,
			TDCollisionHandlers::CollidePlaneBox,
			TDCollisionHandlers::CollidePlaneConvex,
			TDCollisionHandlers::InvalidCollisonPair,
		},
		//EShapeType::eCapsule
		{
			0,
			0,
			TDCollisionHandlers::CollideCapsuleCapsule,
			TDCollisionHandlers::CollideCapsuleBox,
			TDCollisionHandlers::CollideCapsuleConvex,
			TDCollisionHandlers::CollideCapsuleMesh,
		},
		//EShapeType::eBox
		{
			0,
			0,
			0,
			TDCollisionHandlers::CollideBoxBox,
			TDCollisionHandlers::CollideBoxConvex,
			TDCollisionHandlers::CollideBoxMesh,
		},
		//EShapeType::eConvexMesh
		{
			0,
			0,
			0,
			0,
			TDCollisionHandlers::CollideConvexConvex,
			TDCollisionHandlers::CollideConvexMesh,
		},
		//EShapeType::eTriangleMesh
		{
			0,
			0,
			0,
			0,
			TDCollisionHandlers::InvalidCollisonPair,
			TDCollisionHandlers::InvalidCollisonPair,
		}
	};
}