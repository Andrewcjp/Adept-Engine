#pragma once
#include "Physics/GenericRigidBody.h"
class PhysxCollider : public GenericCollider
{
public:
	PhysxCollider();
	~PhysxCollider();
	bool IsDead = false;
private:
	
};

