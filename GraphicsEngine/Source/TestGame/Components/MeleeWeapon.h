#pragma once
#include "Weapon.h"
class MeleeWeapon : public Weapon
{
public:
	MeleeWeapon();
	~MeleeWeapon();
	virtual void Fire() override;
};

