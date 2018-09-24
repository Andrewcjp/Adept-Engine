#include "GenericRigidBody.h"



GenericRigidBody::GenericRigidBody()
{

}


GenericRigidBody::~GenericRigidBody()
{}

glm::vec3 GenericRigidBody::GetPosition()
{
	return glm::vec3();

}

glm::quat GenericRigidBody::GetRotation()
{
	return glm::quat();
}

void GenericRigidBody::AddTorque(glm::vec3 torque)
{

}

void GenericRigidBody::AddForce(glm::vec3 force)
{}

glm::vec3 GenericRigidBody::GetLinearVelocity()
{
	return glm::vec3();
}

