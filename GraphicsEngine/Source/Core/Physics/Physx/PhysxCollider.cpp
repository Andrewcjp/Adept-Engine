#include "Stdafx.h"
#include "PhysxCollider.h"


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
