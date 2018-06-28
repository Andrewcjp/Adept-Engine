#pragma once
#include "EngineHeader.h"
class TGExtraComponentRegister :
	public ExtraComponentRegister
{
public:
	TGExtraComponentRegister();
	~TGExtraComponentRegister();
	Component* CreateExtraComponent(int id)override;
};

 