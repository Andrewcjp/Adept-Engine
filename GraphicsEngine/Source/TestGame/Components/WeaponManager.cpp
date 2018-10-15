#include "WeaponManager.h"
#include "Editor/EditorWindow.h"
#include "Core/Input/Input.h"
#include "Weapon.h"

WeaponManager::WeaponManager()
{}


WeaponManager::~WeaponManager()
{}

void WeaponManager::InitComponent()
{

}

Weapon * WeaponManager::GetCurrentWeapon()
{
	return Weapons[CurrentIndex];
}

void WeaponManager::SwitchWeaponUp(bool Direction)
{
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

void WeaponManager::Update(float delta)
{
	if (EditorWindow::GetInstance()->IsEditorEjected())
	{
		return;
	}
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
}
