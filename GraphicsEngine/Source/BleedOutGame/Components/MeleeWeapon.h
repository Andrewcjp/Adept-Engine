#pragma once
#include "Weapon.h"

class ColliderComponent;
class MeleeWeapon : public Weapon
{
public:
	MeleeWeapon();
	~MeleeWeapon();
	virtual bool Fire() override;
	ColliderComponent* Collider = nullptr;

	virtual void Update(float delta) override;

	virtual void OnTrigger(CollisonData data) override;

	virtual void BeginPlay() override;
	float AttackDamage = 50.0f;
private:
	float AttackLength = 0.2f;
	float CurrentAttackTime = 0.0f;
	float AttackCooldown = 0.5f;
	float CurrentAttackCoolDown = 0.0f;
};

