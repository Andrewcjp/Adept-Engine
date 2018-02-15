#include "TGExtraComponentRegister.h"
#include "TGComp.h"


TGExtraComponentRegister::TGExtraComponentRegister()
{
	Ids.push_back(1);
}


TGExtraComponentRegister::~TGExtraComponentRegister()
{}

Component * TGExtraComponentRegister::CreateExtraComponent(int id)
{
	return new TGComp();
	if (id == 0)
	{
		return new TGComp();
	}
	return nullptr;
}
