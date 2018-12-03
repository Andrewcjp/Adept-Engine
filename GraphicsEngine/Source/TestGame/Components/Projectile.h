#pragma once
#include "Core/Components/Core_Components_inc.h"
class Projectile : public Component
{
public:
	Projectile();
	Projectile(GameObject* owner)
	{
		Owner = owner;
	}
	~Projectile();

	// Inherited via Component
	virtual void InitComponent() override;
	void SetDamage(float Damage) { DamageValue = Damage; }
	float GetDamage()const { return DamageValue; }
	void OnCollide(CollisonData data) override;
	virtual void OnTrigger(CollisonData data) override;


	virtual void Update(float delta) override;

private:
	float DamageValue = 0.0f;
	GameObject* Owner = nullptr;
	float LifeTime = 0.0f;
};

