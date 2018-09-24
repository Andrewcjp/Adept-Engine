#pragma once
#if PHYSX_ENABLED
#include "glm\glm.hpp"
#include "glm\gtx\quaternion.hpp"
#include "Physics/GenericRigidBody.h"
#include "physx\PxPhysicsAPI.h"
class PhysxRigidbody : public GenericRigidBody
{
public:
	~PhysxRigidbody();
	PhysxRigidbody(physx::PxRigidDynamic * Rigid);
	CORE_API glm::vec3 GetPosition();
	CORE_API glm::quat GetRotation();
	CORE_API void AddTorque(glm::vec3);
	CORE_API void AddForce(glm::vec3);
	CORE_API glm::vec3 GetLinearVelocity();
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
#endif
