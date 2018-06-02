#include "TGExtraComponentRegister.h"
#include "TGComp.h"
#include "PhysicsThrowerComponent.h"

TGExtraComponentRegister::TGExtraComponentRegister():ExtraComponentRegister()
{
	Ids.push_back(1);
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
	return nullptr;
}
