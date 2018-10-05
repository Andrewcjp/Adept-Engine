#include "Weapon.h"
#include "Core/Components/Core_Components_inc.h"


Weapon::Weapon()
{}


Weapon::~Weapon()
{}

void Weapon::InitComponent()
{

}

void Weapon::SetCurrentSettings(WeaponSettings NewSettings)
{
	CurrentSettings = NewSettings;
}

void Weapon::Fire()
{

}
