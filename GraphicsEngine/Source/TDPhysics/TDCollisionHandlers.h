#pragma once
#include "Stdafx.h"
#include "TDShape.h"
namespace TD
{
#define CollisionHandlerArgs TDShape* A, TDShape* B, ContactData* contactbuffer
	const int MAX_CONTACT_POINTS_COUNT = 50;
	struct ContactData
	{
		glm::vec3 ContactPoints[MAX_CONTACT_POINTS_COUNT];
		bool Blocking = false;
		glm::vec3 Direction = glm::vec3();
		float depth = 0.0f;
		int ContactCount = 0;
		void Contact(glm::vec3 position, glm::vec3 normal, float seperation)
		{
			Blocking = true;
			ContactPoints[ContactCount] = position;
			ContactCount++;
			Direction = normal;
			depth = seperation;
		}
	};

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
	};
#define InterSectionArgs TDShape* Shape,glm::vec3 Origin,glm::vec3 Dir,float distance
	struct TDIntersectionHandlers
	{
		static bool IntersectSphere(InterSectionArgs);
		static bool IntersectPlane(InterSectionArgs);
		static bool IntersectCapsule(InterSectionArgs);
		static bool IntersectBox(InterSectionArgs);
		static bool IntersectConvex(InterSectionArgs);
		static bool IntersectMesh(InterSectionArgs);
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