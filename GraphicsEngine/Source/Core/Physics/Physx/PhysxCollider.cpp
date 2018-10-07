#include "Stdafx.h"
#include "PhysxCollider.h"
#include "PhysxRigidbody.h"
#include "Core/Components/RigidbodyComponent.h"
RigidBody * PhysxCollider::GetOwner()
{
	return Owner;
}

PhysxCollider::PhysxCollider()
{}


PhysxCollider::~PhysxCollider()
{}

void PhysxCollider::SetOwner(RigidBody * newowner)
{
	Owner = newowner;
}
GameObject * PhysxCollider::GetGameObject()
{
	if (Owner != nullptr && Owner->GetOwnerComponent() != nullptr && Owner->GetOwnerComponent()->GetOwner() != nullptr)
	{
		return Owner->GetOwnerComponent()->GetOwner();
	}
	return nullptr;
}