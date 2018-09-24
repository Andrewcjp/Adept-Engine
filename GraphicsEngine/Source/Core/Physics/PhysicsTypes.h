#pragma once
#include "Physics/Physics_fwd.h"
namespace EShapeType
{
	enum Type
	{
		eSPHERE,
		ePLANE,
		eCAPSULE,
		eBOX,
		eCONVEXMESH,
		eTRIANGLEMESH,
	};
}

typedef struct _RayHit
{
	glm::vec3 position = glm::vec3();
	RigidBody* HitBody = nullptr;
	bool StartInside = false;
}RayHit;