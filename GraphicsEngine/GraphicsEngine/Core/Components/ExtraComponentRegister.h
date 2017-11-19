#pragma once
#include "Component.h"
class ExtraComponentRegister
{
public:
	ExtraComponentRegister();
	~ExtraComponentRegister();
	virtual Component* CreateExtraComponent(int id);
	virtual std::vector<int> GetExtraCompIds();
protected:
	std::vector<int> Ids;
};

