#pragma once
#include "Core\Components\Component.h"
class PhysicsThrowerComponent :public Component
{
public:
	PhysicsThrowerComponent();
	virtual ~PhysicsThrowerComponent();

	// Inherited via Component
	virtual void BeginPlay() override;
	virtual void Update(float delta) override;

	void CreateStackAtPoint();

	void FireAtScene();

	// Inherited via Component
	virtual void InitComponent() override;

	// Inherited via Component
	virtual void Serialise(rapidjson::Value & v) override;
	virtual void Deserialise(rapidjson::Value & v) override;
private:
	float CurrentForce = 1.0f;
};

