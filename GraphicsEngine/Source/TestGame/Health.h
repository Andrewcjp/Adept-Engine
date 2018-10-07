#pragma once
#include "Core/Components/Component.h"
class Health : public Component
{
public:
	Health();
	~Health();
	void TakeDamage(float amt);
private:
	float MaxHealth = 100.0f;
	float CurrentHealth = 100.0f;

	// Inherited via Component
	virtual void InitComponent() override;

};

