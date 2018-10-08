#pragma once
#include "Physics/GenericRigidBody.h"
class PhysxCollider : public GenericCollider
{
public:
	RigidBody* GetOwner();
	PhysxCollider();
	~PhysxCollider();
	void SetOwner(RigidBody* newowner);
	CORE_API class GameObject* GetGameObject();
	bool IsDead = false;
private:
	RigidBody* Owner = nullptr;
};

