#include "WeaponManager.h"
#include "Editor/EditorWindow.h"
#include "Core/Input/Input.h"
#include "Weapon.h"
#include "MeleeWeapon.h"

WeaponManager::WeaponManager()
{}


WeaponManager::~WeaponManager()
{}

void WeaponManager::InitComponent()
{

}

void WeaponManager::BeginPlay()
{
	for (int i = 0; i < MAX_WEAPON_COUNT; i++)
	{
		Weapons[i]->SetState(false);
	}
	Weapons[0]->SetState(true);
}

void WeaponManager::AddAmmo(Weapon::WeaponType Type, int amount)
{
	if (Type < Weapon::Limit)
	{
		Weapons[Type]->AddAmmo(amount);
	}
}

Weapon * WeaponManager::GetCurrentWeapon()
{
	return Weapons[CurrentIndex];
}

void WeaponManager::SwitchWeaponUp(bool Direction)
{
	if (CurrentIndex == 0 && !Direction)
	{
		CurrentIndex = MAX_WEAPON_COUNT - 1;
		return;
	}
	CurrentIndex += Direction ? 1 : -1;
	CurrentIndex %= MAX_WEAPON_COUNT;
}

void WeaponManager::SwitchWeapon(int index)
{
	if (index < MAX_WEAPON_COUNT && index >= 0)
	{
		CurrentIndex = index;
	}
}

void WeaponManager::TryToMelee()
{
	Melee->Fire();
}

std::string WeaponManager::GetCurrentWeaponinfoString()
{
	std::stringstream ss;
	ss << Weapons[CurrentIndex]->GetCurrentAmmo() << " / " << Weapons[CurrentIndex]->GetCurrentSettings().MaxAmmoCount;
	return ss.str();
}

void WeaponManager::Update(float delta)
{
#if WITH_EDTIOR
	if (EditorWindow::GetInstance()->IsEditorEjected())
	{
		return;
	}
#endif
	if (Input::GetMouseButtonDown(0))
	{
		GetCurrentWeapon()->Fire();
	}
	if (Input::GetMouseWheelUp())
	{
		Weapons[CurrentIndex]->SetState(false);
		SwitchWeaponUp(true);
		Weapons[CurrentIndex]->SetState(true);
	}
	if (Input::GetMouseWheelDown())
	{
		Weapons[CurrentIndex]->SetState(false);
		SwitchWeaponUp(false);
		Weapons[CurrentIndex]->SetState(true);
	}
	if (Input::GetKeyDown('F'))
	{
		TryToMelee();
	}
}
