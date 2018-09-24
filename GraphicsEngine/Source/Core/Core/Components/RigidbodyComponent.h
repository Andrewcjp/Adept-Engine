#pragma once
#include "Component.h"
#include "Physics/Physics_fwd.h"
class RigidbodyComponent :
	public Component
{
public:
	CORE_API RigidbodyComponent();
	~RigidbodyComponent();

	// Inherited via Component
	void BeginPlay() override;
	void Update(float delta) override;
	void FixedUpdate(float delta) override;
	virtual void InitComponent() override;
private:
	RigidBody* actor;

	// Inherited via Component



};

