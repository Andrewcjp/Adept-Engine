#include "WeaponManager.h"
#include "Editor/EditorWindow.h"
#include "Core/Input/Input.h"
#include "Weapon.h"
#include "MeleeWeapon.h"
#include "Audio/AudioEngine.h"

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
	if (Type < MAX_WEAPON_COUNT)
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
	AudioEngine::PostEvent("Change_Weapon");
	Weapons[CurrentIndex]->SetState(false);
	if (CurrentIndex == 0 && !Direction)
	{
		CurrentIndex = MAX_WEAPON_COUNT - 1;
		Weapons[CurrentIndex]->SetState(true);
		return;
	}
	CurrentIndex += Direction ? 1 : -1;
	CurrentIndex %= MAX_WEAPON_COUNT;
	Weapons[CurrentIndex]->SetState(true);
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
void WeaponManager::SetWeaponActive(int index)
{
	if (CurrentIndex == index)
	{
		return;
	}
	Weapons[CurrentIndex]->SetState(false);
	CurrentIndex = index;
	Weapons[CurrentIndex]->SetState(true);
	AudioEngine::PostEvent("Change_Weapon", GetOwner());
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
		SwitchWeaponUp(true);
	}
	if (Input::GetMouseWheelDown())
	{
		SwitchWeaponUp(false);
	}
	if (Input::GetKeyDown('F'))
	{
		TryToMelee();
	}
	if (Input::GetKeyDown('1'))
	{
		SetWeaponActive(0);
	}
	if (Input::GetKeyDown('2'))
	{
		SetWeaponActive(1);
	}
#if USE_SHOTGUN
	if (Input::GetKeyDown('3'))
	{
		SetWeaponActive(2);
	}
#endif
}
