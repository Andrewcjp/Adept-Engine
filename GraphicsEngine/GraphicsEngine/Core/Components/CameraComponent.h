#pragma once
#include "Component.h"
class CameraComponent :
	public Component
{
public:
	CameraComponent();
	~CameraComponent();

	// Inherited via Component
	virtual void BeginPlay() override;
	virtual void Update(float delta) override;
};

