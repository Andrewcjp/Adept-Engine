#pragma once
#include "EngineGlobals.h"
#include "glm\glm.hpp"
#include "glm\gtx\quaternion.hpp"

class RigidBody
{
public:
	RigidBody();
	~RigidBody();
	virtual glm::vec3 GetPosition();
	virtual glm::quat GetRotation();
	virtual void AddTorque(glm::vec3);
	virtual void AddForce(glm::vec3);
	virtual glm::vec3 GetLinearVelocity();
private:

};

