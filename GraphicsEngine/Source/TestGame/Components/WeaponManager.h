#pragma once
const int MAX_WEAPON_COUNT = 3;
class Weapon;
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
private:
	
	void Update(float delta) override;
	int CurrentIndex = 0;
};

