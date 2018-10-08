#pragma once
#include "Core/Components/Component.h"
class Health : public Component
{
public:
	Health();
	~Health();
	void TakeDamage(float amt);
	float MaxHealth = 100.0f;
private:
	
	float CurrentHealth = 100.0f;

	// Inherited via Component
	virtual void SceneInitComponent() override;


	// Inherited via Component
	virtual  void InitComponent() override;

};

