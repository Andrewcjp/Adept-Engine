#include "TGExtraComponentRegister.h"
#include "TGComp.h"


TGExtraComponentRegister::TGExtraComponentRegister()
{
	Ids.push_back(1);
}


TGExtraComponentRegister::~TGExtraComponentRegister()
{
}

Component * TGExtraComponentRegister::CreateExtraComponent(int id)
{
	if (id == 1)
	{
		return new TGComp();
	}
	return nullptr;
}
