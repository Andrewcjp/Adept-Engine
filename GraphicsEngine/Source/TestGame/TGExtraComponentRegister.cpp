#include "TGExtraComponentRegister.h"
#include "TGComp.h"
#include "PhysicsThrowerComponent.h"
#include "TestPlayer.h"
TGExtraComponentRegister::TGExtraComponentRegister():ExtraComponentRegister()
{
	Ids.push_back(1);
	Ids.push_back(2);
}


TGExtraComponentRegister::~TGExtraComponentRegister()
{}

Component * TGExtraComponentRegister::CreateExtraComponent(int id)
{
	if (id == 0)
	{
		return new TGComp();
	}
	if (id == 1)
	{
		return new PhysicsThrowerComponent();
	}
	if (id == 2)
	{
		return new TestPlayer();
	}
	return nullptr;
}
