#pragma once
#include "Component.h"
class LightComponent :
	public Component
{
public:
	LightComponent();
	~LightComponent();

	// Inherited via Component
	virtual void BeginPlay() override;
	virtual void Update(float delta) override;
};

