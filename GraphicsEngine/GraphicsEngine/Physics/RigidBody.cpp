#include "RigidBody.h"



RigidBody::RigidBody()
{

}


RigidBody::~RigidBody()
{
}

glm::vec3 RigidBody::GetPosition() 
{
	return glm::vec3();
}

glm::quat RigidBody::GetRotation() 
{
	return glm::quat();
}

void RigidBody::AddTorque(glm::vec3)
{
}

void RigidBody::AddForce(glm::vec3)
{
}

glm::vec3 RigidBody::GetLinearVelocity()
{
	return glm::vec3();
}

