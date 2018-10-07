#pragma once
#include "EngineHeader.h"
namespace ComponentTypes
{
	enum TYPE
	{
		TGComp,
		PhysicsThrowerComponent,
		TestPlayer,
		Weapon,
		Projectile,
		Health,
		Limit
	};
}
class TGExtraComponentRegister :
	public ExtraComponentRegister
{
public:
	TGExtraComponentRegister();
	~TGExtraComponentRegister();
	Component* CreateExtraComponent(int id)override;
};

 