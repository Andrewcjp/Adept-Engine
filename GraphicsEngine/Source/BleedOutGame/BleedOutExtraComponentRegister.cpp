#include "BleedOutExtraComponentRegister.h"
#include "Components/BleedOutPlayer.h"
#include "Components/Weapon.h"
#include "Components/Projectile.h"
#include "Components/Health.h"
#include "Components/WeaponManager.h"
BleedOutExtraComponentRegister::BleedOutExtraComponentRegister() :ExtraComponentRegister()
{
	for (int i = 0; i < ComponentTypes::Limit; i++)
	{
		Ids.push_back(i);
	}
}


BleedOutExtraComponentRegister::~BleedOutExtraComponentRegister()
{}

Component * BleedOutExtraComponentRegister::CreateExtraComponent(int id)
{
	switch (id)
	{
	case ComponentTypes::TGComp:
		return nullptr;
	case ComponentTypes::PhysicsThrowerComponent:
		return nullptr;
	case ComponentTypes::TestPlayer:
		return new BleedOutPlayer();
	case ComponentTypes::Weapon:
		return new Weapon();
	case ComponentTypes::Projectile:
		return new Projectile();
	case ComponentTypes::Health:
		return new Health();
	case ComponentTypes::WeaponManager:
		return new WeaponManager();
	}
	return nullptr;
}
