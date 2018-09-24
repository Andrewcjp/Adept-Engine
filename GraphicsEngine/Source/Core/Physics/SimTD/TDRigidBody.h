#pragma once
#if TDSIM_ENABLED
#include "glm\glm.hpp"
#include "glm\gtx\quaternion.hpp"
class TDRigidBody
{
public:
	TDRigidBody();
	~TDRigidBody();
	CORE_API glm::vec3 GetPosition();
	CORE_API glm::quat GetRotation();
	CORE_API void AddTorque(glm::vec3);
	CORE_API void AddForce(glm::vec3);
	CORE_API glm::vec3 GetLinearVelocity();
private:
};

#endif