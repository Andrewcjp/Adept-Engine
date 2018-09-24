#include "TDRigidBody.h"
#if TDSIM_ENABLED


TDRigidBody::TDRigidBody()
{

}


TDRigidBody::~TDRigidBody()
{}

glm::vec3 TDRigidBody::GetPosition()
{
	return glm::vec3();

}

glm::quat TDRigidBody::GetRotation()
{
	return glm::quat();
}

void TDRigidBody::AddTorque(glm::vec3 torque)
{

}

void TDRigidBody::AddForce(glm::vec3 force)
{}

glm::vec3 TDRigidBody::GetLinearVelocity()
{
	return glm::vec3();
}

#endif