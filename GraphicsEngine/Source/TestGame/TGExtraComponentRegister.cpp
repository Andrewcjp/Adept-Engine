#include "TGExtraComponentRegister.h"
#include "Components/TGComp.h"
#include "Components/PhysicsThrowerComponent.h"
#include "Components/TestPlayer.h"
#include "Components/Weapon.h"
#include "Components/Projectile.h"
#include "Components/Health.h"
TGExtraComponentRegister::TGExtraComponentRegister() :ExtraComponentRegister()
{
	for (int i = 0; i < ComponentTypes::Limit; i++)
	{
		Ids.push_back(i);
	}
}


TGExtraComponentRegister::~TGExtraComponentRegister()
{}

Component * TGExtraComponentRegister::CreateExtraComponent(int id)
{
	switch (id)
	{
	case ComponentTypes::TGComp:
		return new TGComp();
	case ComponentTypes::PhysicsThrowerComponent:
		return new PhysicsThrowerComponent();
	case ComponentTypes::TestPlayer:
		return new TestPlayer();
	case ComponentTypes::Weapon:
		return new Weapon();
	case ComponentTypes::Projectile:
		return new Projectile();
	case ComponentTypes::Health:
		return new Health();
	}
	return nullptr;
}
