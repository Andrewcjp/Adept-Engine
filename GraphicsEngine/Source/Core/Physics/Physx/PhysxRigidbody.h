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
	glm::vec3 GetPosition()const;
	glm::quat GetRotation()const;
	void SetPositionAndRotation(glm::vec3 pos, glm::quat rot);
	CORE_API void AddTorque(glm::vec3);
	CORE_API void AddForce(glm::vec3, EForceMode::Type Mode = EForceMode::AsForce);
	glm::vec3 GetLinearVelocity() const;
	CORE_API void SetLinearVelocity(glm::vec3 velocity);
	glm::vec3 GetAngularVelocity()const;
	CORE_API void SetAngularVelocity(glm::vec3 velocity);
	void AttachCollider(Collider* col);
	void SetPhysicalMaterial(PhysicalMaterial* newmat);

	void UpdateBodyState() override;

	physx::PxRigidActor* GetActor() { return CommonActorPtr; }
	void InitBody();
private:
	std::vector<Collider*> AttachedColliders;
	physx::PxTriangleMesh * GenerateTriangleMesh(std::string Filename, glm::vec3 scale);
	physx::PxConvexMesh * GenerateConvexMesh(std::string Filename, glm::vec3 scale);
	physx::PxRigidDynamic*	Dynamicactor = nullptr;
	physx::PxRigidStatic*	StaticActor = nullptr;
	physx::PxRigidActor*	CommonActorPtr = nullptr;

	std::vector<physx::PxShape*> Shapes;
	Transform transform;
	PhysicalMaterial* PhysicsMat = nullptr;
	physx::PxMaterial* PMaterial = nullptr;
};
#endif
