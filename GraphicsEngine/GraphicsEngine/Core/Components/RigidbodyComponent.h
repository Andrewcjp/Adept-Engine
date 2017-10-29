#pragma once
#include "Component.h"
class RigidbodyComponent :
	public Component
{
public:
	RigidbodyComponent();
	~RigidbodyComponent();

	// Inherited via Component
	virtual void BeginPlay() override;
	virtual void Update(float delta) override;
};

