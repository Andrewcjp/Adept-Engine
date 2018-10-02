#pragma once
#if PHYSX_ENABLED
#include "glm\glm.hpp"
#include "glm\gtx\quaternion.hpp"
#include "Physics/GenericRigidBody.h"
#include "Physics/PhysicsTypes.h"
#include "physx\PxPhysicsAPI.h"
#include "Core/Transform.h"
class PhysxRigidbody : public GenericRigidBody
{
public:
	~PhysxRigidbody();
	PhysxRigidbody(EBodyType::Type type, Transform InitalPosition);
	CORE_API glm::vec3 GetPosition();
	CORE_API glm::quat GetRotation();
	void SetPositionAndRotation(glm::vec3 pos, glm::quat rot);
	CORE_API void AddTorque(glm::vec3);
	CORE_API void AddForce(glm::vec3, EForceMode::Type Mode = EForceMode::AsForce);
	CORE_API glm::vec3 GetLinearVelocity();
	CORE_API void SetLinearVelocity(glm::vec3 velocity);
	void AttachCollider(Collider* col);
	void SetPhysicalMaterial(PhysicalMaterial* newmat);

	void UpdateFlagStates() override;

	void InitBody();
private:

	physx::PxRigidDynamic*	Dynamicactor = nullptr;
	physx::PxRigidStatic*	StaticActor = nullptr;
	physx::PxRigidActor*	CommonActorPtr = nullptr;
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
	physx::PxQuat GLMtoPXQuat(glm::quat val)
	{
		return physx::PxQuat(val.x, val.y, val.z, val.w);
	}
	std::vector<physx::PxShape*> Shapes;
	Transform transform;
	PhysicalMaterial* PhysicsMat = nullptr;
	physx::PxMaterial* PMaterial = nullptr;
};
#endif
