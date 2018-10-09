#include "GenericRigidBody.h"
#include "Core/Components/Component.h"
#include "Physics/PhysicsEngine.h"
GenericRigidBody::GenericRigidBody(EBodyType::Type type)
{
	BodyType = type;
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

void GenericRigidBody::AddForce(glm::vec3 force, EForceMode::Type Mode)
{
	
}


glm::vec3 GenericRigidBody::GetLinearVelocity()
{
	return glm::vec3();
}

void GenericRigidBody::AttachCollider(Collider * col)
{}

void GenericRigidBody::SetLockFlags(BodyInstanceData data)
{
	LockData = data;
	UpdateFlagStates();
}

BodyInstanceData GenericRigidBody::GetLockFlags()
{
	return LockData;
}

GenericCollider::GenericCollider()
{}

GenericCollider::~GenericCollider()
{}
GameObject * GenericCollider::GetGameObject()
{
	if (Owner != nullptr && Owner->GetOwnerComponent() != nullptr && Owner->GetOwnerComponent()->GetOwner() != nullptr)
	{
		return Owner->GetOwnerComponent()->GetOwner();
	}
	return nullptr;
}

void GenericCollider::SetOwner(RigidBody * newowner)
{
	Owner = newowner;
}

RigidBody * GenericCollider::GetOwner()
{
	return Owner;
}