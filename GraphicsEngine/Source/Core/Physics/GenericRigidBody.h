#pragma once

#include "glm\glm.hpp"
#include "glm\gtx\quaternion.hpp"
#include "PhysicsTypes.h"
struct BodyInstanceData
{
	bool LockXRot = false;
	bool LockYRot = false;
	bool LockZRot = false;
	bool LockXPosition = false;
	bool LockYPosition = false;
	bool LockZPosition = false;
	float AngularDamping = 0.0f;
	float LinearDamping = 0.05f;
	float Mass = 1.0f;
	bool UseAutoMass = false;
};
class GenericRigidBody
{
public:
	GenericRigidBody(EBodyType::Type type);
	~GenericRigidBody();
	CORE_API glm::vec3 GetPosition();
	CORE_API glm::quat GetRotation();
	CORE_API void AddTorque(glm::vec3);
	CORE_API void AddForce(glm::vec3 force, EForceMode::Type Mode = EForceMode::AsForce);
	CORE_API glm::vec3 GetLinearVelocity();
	void AttachCollider(Collider* col);
	CORE_API void SetLockFlags(BodyInstanceData data);
	CORE_API BodyInstanceData GetLockFlags();

protected:
	virtual void UpdateFlagStates() {};
	EBodyType::Type BodyType;
	BodyInstanceData LockData;
};

class GenericCollider
{
public:
	GenericCollider();
	~GenericCollider();
	std::vector<ShapeElem*> Shapes;
protected:

};

