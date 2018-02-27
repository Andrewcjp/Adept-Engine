#pragma once
#include "RigidBody.h"
#if 0

#include "physx\PxPhysicsAPI.h"

class PhxRigidBody :
	public RigidBody
{
public:
	PhxRigidBody();
	PhxRigidBody(physx::PxRigidDynamic * Rigid);
	glm::vec3 GetPosition() override;
	glm::quat GetRotation()override;
	void AddTorque(glm::vec3)override;
	void AddForce(glm::vec3)override;
	glm::vec3 GetLinearVelocity()override;
	~PhxRigidBody();
private:
	physx::PxRigidDynamic* actor;
	glm::vec3 PXvec3ToGLM(physx::PxVec3 val) {
		return glm::vec3(val.x, val.y, val.z);
	}
	glm::quat PXquatToGLM(physx::PxQuat val) {
		return glm::quat(val.x, val.y, val.z, val.w);
	}
	physx::PxVec3 GLMtoPXvec3(glm::vec3 val) {
		return physx::PxVec3(val.x, val.y, val.z);
	}
};
#endif

