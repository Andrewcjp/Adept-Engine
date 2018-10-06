#pragma once
#include "Physics/GenericRigidBody.h"
class PhysxCollider : public GenericCollider
{
public:
	RigidBody* GetOwner();
	PhysxCollider();
	~PhysxCollider();
	void SetOwner(RigidBody* newowner);
private:
	RigidBody* Owner = nullptr;
};

