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
	bool Gravity = true;
};
class Component;
class GenericRigidBody
{
public:
	GenericRigidBody(EBodyType::Type type);
	virtual ~GenericRigidBody();
	glm::vec3 GetPosition();
	glm::quat GetRotation();
	void AddTorque(glm::vec3);
	/**
	*\brief Adds a force at the bodies root 
	*\param Force to add
	*\param Force addition mode
	*/
	void AddForce(glm::vec3 force, EForceMode::Type Mode = EForceMode::AsForce);
	/**
	*\brief Returns the Linear velocity of the body from the last timestep
	*/
	glm::vec3 GetLinearVelocity();
	/**
	*\brief Sets the linear velocity of this body for the next timestep
	*/
	void SetLinearVelocity(glm::vec3 velocity);
	/**
	*\brief Returns the angular velocity of the body from the last timestep 
	*/
	glm::vec3 GetAngularVelocity();
	/**
	*\brief Sets the Angular velocity of this body for the next timestep
	*/
	void SetAngularVelocity(glm::vec3 velocity);

	void SetGravity(bool active);
	void AttachCollider(Collider* col);

	void SetBodyData(BodyInstanceData data);
	BodyInstanceData& GetBodyData();

	Component* GetOwnerComponent() { return OwningComponent; }
	void SetOwnerComponent(Component* newowner) { OwningComponent = newowner; }
protected:
	virtual void UpdateBodyState() {};
	EBodyType::Type BodyType;
	BodyInstanceData BodyData;
	Component* OwningComponent = nullptr;
};

class GenericCollider
{
public:
	GenericCollider();
	virtual ~GenericCollider();
	virtual class GameObject * GetGameObject();
	std::vector<ShapeElem*> Shapes;
	CORE_API void SetOwner(RigidBody* newowner);
	CORE_API RigidBody * GetOwner();
protected:
	RigidBody* Owner = nullptr;
};

