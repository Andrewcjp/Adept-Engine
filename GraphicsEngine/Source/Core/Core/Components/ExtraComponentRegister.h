#pragma once
#include "Component.h"

class ExtraComponentRegister
{
public:
	CORE_API  ExtraComponentRegister();
	CORE_API  ~ExtraComponentRegister();
	CORE_API  virtual Component* CreateExtraComponent(int id);
	CORE_API  virtual std::vector<int> GetExtraCompIds();
protected:
	std::vector<int> Ids;
};

