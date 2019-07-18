#include "GameModuleSelector.h"
#include "Core/Module/GameModuleSelector.generated.h"
std::string GameModuleSelector::GetGameModuleName()
{
	return GAMEMODULENAME;
}
#ifndef GAMEMODULENAME
#error "GAMEMODULENAME Undefined header tool have not been run"
#endif