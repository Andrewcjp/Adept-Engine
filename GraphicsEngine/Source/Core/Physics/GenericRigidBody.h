#pragma once
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
	PhysicalMaterial* Mat = nullptr;
};

class Component;
class ColliderComponent;
namespace physx { class PxShape; }
/*! This class is used as the compile time base class for the rigid body class as only one physics engine can be used at once*/
class GenericRigidBody
{
public:
	GenericRigidBody(EBodyType::Type type);
	virtual ~GenericRigidBody();
	/**
	*\return The bodies current root position
	*/
	glm::vec3 GetPosition()const;
	/**
	*\return The bodies current rotation
	*/
	glm::quat GetRotation()const;
	/**
	*\brief Adds a Torque around the bodies root
	*\param Torque to add
	*\param Force addition mode
	*/
	void AddTorque(glm::vec3 Torque);
	/**
	*\brief Adds a force at the bodies root
	*\param Force to add
	*\param Force addition mode
	*/
	void AddForce(glm::vec3 force, EForceMode::Type Mode = EForceMode::AsForce);
	/**
	*\brief Returns the Linear velocity of the body from the last timestep
	*/
	glm::vec3 GetLinearVelocity() const;
	/**
	*\brief Sets the linear velocity of this body for the next timestep
	*/
	void SetLinearVelocity(glm::vec3 velocity);
	/**
	*\brief Returns the angular velocity of the body from the last timestep
	*/
	glm::vec3 GetAngularVelocity()const;
	/**
	*\brief Sets the Angular velocity of this body for the next timestep
	*/
	void SetAngularVelocity(glm::vec3 velocity);
	/**
	*\brief Returns the bodies Inertia Tensor
	*/
	glm::vec3 GetInertiaTensor()const;
	/**
	*\brief Sets the bodies Inertia Tensor
	*/
	void SetInertiaTensor(float newtensor);

	void SetGravity(bool active);
	void AttachCollider(Collider* col);

	void SetBodyData(BodyInstanceData data);
	BodyInstanceData& GetBodyData();

	Component* GetOwnerComponent() const { return OwningComponent; }
	void SetOwnerComponent(Component* newowner) { OwningComponent = newowner; }
	PhysicalMaterial* PhysicsMat = nullptr;
protected:
	virtual void UpdateBodyState() {};
	EBodyType::Type BodyType;
	BodyInstanceData BodyData;
	Component* OwningComponent = nullptr;

};
namespace TD { class TDShape; };
/*! This class is used as the compile time base class for the rigid body class as only one physics engine can be used at once*/
class GenericCollider
{
public:
	GenericCollider();
	virtual ~GenericCollider();
	virtual class GameObject * GetGameObject();
	std::vector<ShapeElem*> Shapes;
	CORE_API void SetOwner(RigidBody* newowner);
	CORE_API RigidBody * GetOwner();
	bool IsTrigger = false;
#if PHYSX_ENABLED
	physx::PxShape* Shape = nullptr;
#elif TDSIM_ENABLED
	TD::TDShape* Shape = nullptr;
#endif
	void SetEnabled(bool state);
	ColliderComponent* ComponentOwner = nullptr;
protected:
	RigidBody* Owner = nullptr;
	
};

