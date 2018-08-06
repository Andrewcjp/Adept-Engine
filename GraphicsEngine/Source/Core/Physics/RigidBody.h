#pragma once

#include "glm\glm.hpp"
#include "glm\gtx\quaternion.hpp"

#include "physx\PxPhysicsAPI.h"
class RigidBody
{
public:
	RigidBody();
	~RigidBody();
	RigidBody(physx::PxRigidDynamic * Rigid);
	CORE_API glm::vec3 GetPosition() ;
	glm::quat GetRotation();
	void AddTorque(glm::vec3);
	CORE_API void AddForce(glm::vec3);
	glm::vec3 GetLinearVelocity();
private:
	physx::PxRigidDynamic* actor;
	glm::vec3 PXvec3ToGLM(physx::PxVec3 val)
	{
		return glm::vec3(val.x, val.y, val.z);
	}
	glm::quat PXquatToGLM(physx::PxQuat val)
	{
		return glm::quat(val.x, val.y, val.z, val.w);
	}
	physx::PxVec3 GLMtoPXvec3(glm::vec3 val)
	{
		return physx::PxVec3(val.x, val.y, val.z);
	}

};

