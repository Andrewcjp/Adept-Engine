#pragma once
#include "Stdafx.h"
#include "TDShape.h"
#include "Shapes/TDSphere.h"
#include "Shapes/TDMesh.h"
#include "Shapes/TDBox.h"
#include "Shapes/TDPlane.h"
namespace TD
{
	const int MAX_CONTACT_BUFFER = 50;
	struct ContactData
	{
		glm::vec3 ContractPoints[MAX_CONTACT_BUFFER];
	};
	/*
	\Brief Collsion Handlers for avery possible collision
	*/
	class TDCollisionHandlers
	{
	public:
		static bool CollideSphereSphere(TDSphere* A, TDSphere* B, ContactData* hitbuffer);
		static bool CollideBoxSphere(TDSphere* A, TDBox* B, ContactData* hitbuffer);
		static bool CollideMeshSphere(TDSphere* A, TDMesh* B, ContactData* hitbuffer);
		static bool CollideBoxMesh(TDBox* A, TDMesh* B, ContactData* hitbuffer);
		static bool CollidePlaneSphere(TDPlane* A, TDSphere* B, ContactData* hitbuffer);
	};

}