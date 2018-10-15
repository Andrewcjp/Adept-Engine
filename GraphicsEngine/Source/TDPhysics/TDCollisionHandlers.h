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
	};
	
	class TDCollisionHandlers
	{
	public:
		static bool CollideSphereSphere(CollisionHandlerArgs);
		static bool CollideBoxSphere(CollisionHandlerArgs);
		static bool CollideMeshSphere(CollisionHandlerArgs);
		static bool CollideBoxMesh(CollisionHandlerArgs);
		static bool CollideSpherePlane(CollisionHandlerArgs);
	};
	typedef bool(*ContactMethod)(TDShape*, TDShape*, ContactData*);
	/**
	*\Brief Collsion Handlers for every possible collision
	*/
	static ContactMethod ContactMethodTable[][EShapeType::eLimit] =
	{
		{
			TDCollisionHandlers::CollideSphereSphere,
			TDCollisionHandlers::CollideSpherePlane,			
			TDCollisionHandlers::CollideBoxSphere
		}

	};
}