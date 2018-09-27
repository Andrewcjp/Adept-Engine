#pragma once

#include "glm\glm.hpp"
#include "glm\gtx\quaternion.hpp"
#include "PhysicsTypes.h"
class GenericRigidBody
{
public:
	GenericRigidBody(EBodyType::Type type);
	~GenericRigidBody();
	CORE_API glm::vec3 GetPosition();
	CORE_API glm::quat GetRotation();
	CORE_API void AddTorque(glm::vec3);
	CORE_API void AddForce(glm::vec3);
	CORE_API glm::vec3 GetLinearVelocity();
	void AttachCollider(Collider* col);

protected:
	float AngularDamping = 0.0f;
	float LinearDamping = 0.05f;
	float Mass = 1.0f;
	bool UseAutoMass = false;
	EBodyType::Type BodyType;
};

class GenericCollider
{
public:
	GenericCollider();
	~GenericCollider();
	std::vector<ShapeElem*> Shapes;
protected:
	
};

