#pragma once
#include "Weapon.h"
const int MAX_WEAPON_COUNT = 3;
class Weapon;
class MeleeWeapon;
class WeaponManager :public Component
{
public:
	WeaponManager();
	~WeaponManager();
	void InitComponent() override;

	Weapon* Weapons[MAX_WEAPON_COUNT]; 
	Weapon* GetCurrentWeapon();
	void SwitchWeaponUp(bool Direction);
	void SwitchWeapon(int index);
	void TryToMelee();
	std::string GetCurrentWeaponinfoString();
	MeleeWeapon* Melee = nullptr;

	virtual void BeginPlay() override;
	void AddAmmo(Weapon::WeaponType Type, int amount);
private:
	
	void Update(float delta) override;
	int CurrentIndex = 0;
};

