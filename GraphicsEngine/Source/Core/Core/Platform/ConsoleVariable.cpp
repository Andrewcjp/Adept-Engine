#include "Stdafx.h"
#include "ConsoleVariable.h"
ConsoleVariableManager* ConsoleVariableManager::Instance = nullptr;

ConsoleVariable::ConsoleVariable(std::string name, int DefaultValue, ECVarType::Type cvartype)
{

	CurrentValue = DefaultValue;
	if (cvartype != ECVarType::LaunchOnly)
	{
		ConsoleVariableManager::Get()->ConsoleVars.push_back(this);
	}
	if (cvartype != ECVarType::ConsoleOnly)
	{
		ConsoleVariableManager::Get()->LaunchArgs.push_back(this);
	}

	Name = name;
	std::transform(Name.begin(), Name.end(), Name.begin(), ::tolower);
}


