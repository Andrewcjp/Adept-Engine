
#include "ConsoleVariable.h"
#include "Core/Utils/StringUtil.h"
#include "Logger.h"
ConsoleVariableManager* ConsoleVariableManager::Instance = nullptr;

ConsoleVariable::ConsoleVariable(std::string name, int DefaultValue, ECVarType::Type cvartype, bool needv)
{
	NeedsValue = needv;
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

bool GetValueClean(std::string value,int& outvalue)
{
	if (value.find('-') != -1)	{
		
		outvalue = -1;
		return false;
	}
	outvalue = stoi(value);
	return true;
}

void ConsoleVariableManager::SetupVars(std::string LaunchArgString)
{
	std::transform(LaunchArgString.begin(), LaunchArgString.end(), LaunchArgString.begin(), ::tolower);
	std::vector<std::string> SplitArgs = StringUtils::Split(LaunchArgString, ' ');
	for (ConsoleVariable* CV : Instance->LaunchArgs)
	{
		for (int i = 0; i < SplitArgs.size(); i++)
		{
			std::string Arg = SplitArgs[i];
			if (Arg.compare(CV->GetLaunchName()) == 0)
			{
				if (CV->IsValueVar())
				{
					if (i + 1 < SplitArgs.size())
					{
						int parsedvalue = -1;
						if (GetValueClean(SplitArgs[i + 1], parsedvalue))
						{								
							i++;
						}
						else
						{
							Log::LogMessage("Argument " + CV->GetLaunchName() + " Is missing Value, -1 assumed", Log::Severity::Warning);
						}
						CV->SetValue(parsedvalue);
					}
					else
					{
						Log::LogMessage("Argument " + CV->GetLaunchName() + " Is missing Value, -1 assumed", Log::Severity::Warning);
						CV->SetValue(-1);
					}
				}
				else
				{
					CV->SetValue(1);
				}
			}
		}
	}
}
