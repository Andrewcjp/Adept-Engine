#pragma once

#include "glm\glm.hpp"
#include "glm\gtx\quaternion.hpp"
class GenericRigidBody
{
public:
	GenericRigidBody();
	~GenericRigidBody();
	CORE_API glm::vec3 GetPosition();
	CORE_API glm::quat GetRotation();
	CORE_API void AddTorque(glm::vec3);
	CORE_API void AddForce(glm::vec3);
	CORE_API glm::vec3 GetLinearVelocity();
private:
};

