#include "Stdafx.h"
#include "ConsoleVariable.h"
ConsoleVariableManager* ConsoleVariableManager::Instance = nullptr;

ConsoleVariable::ConsoleVariable(std::string name, int DefaultValue, bool AsLaunchArgs)
{
	
	CurrentValue = DefaultValue;
	ConsoleVariableManager::Get()->ConsoleVars.push_back(this);
	if (AsLaunchArgs)
	{
		ConsoleVariableManager::Get()->LaunchArgs.push_back(this);
	}

	Name = name;
	std::transform(Name.begin(), Name.end(), Name.begin(), ::tolower);
}
