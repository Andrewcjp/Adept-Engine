
#include "ExtraComponentRegister.h"


ExtraComponentRegister::ExtraComponentRegister()
{}


ExtraComponentRegister::~ExtraComponentRegister()
{}

Component * ExtraComponentRegister::CreateExtraComponent(int id)
{
	return nullptr;
}

std::vector<int> ExtraComponentRegister::GetExtraCompIds()
{
	return Ids;
}
