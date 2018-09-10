#include "Stdafx.h"
#include "ConsoleVariable.h"
#include "Core/Utils/StringUtil.h"
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

void ConsoleVariableManager::SetupVars(std::string LaunchArgString)
{
	std::transform(LaunchArgString.begin(), LaunchArgString.end(), LaunchArgString.begin(), ::tolower);
	std::vector<std::string> SplitArgs = StringUtils::Split(LaunchArgString, ' ');
	for (ConsoleVariable* CV : Instance->LaunchArgs)
	{
		for (std::string Arg : SplitArgs)
		{
			if (Arg.compare(CV->GetLaunchName()) == 0)
			{
				CV->SetValue(1);
			}
		}
	}
}
