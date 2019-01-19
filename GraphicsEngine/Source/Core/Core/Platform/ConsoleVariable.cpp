
#include "ConsoleVariable.h"
#include "Core/Utils/StringUtil.h"
#include "Logger.h"
ConsoleVariableManager* ConsoleVariableManager::Instance = nullptr;

ConsoleVariable::ConsoleVariable(std::string name, int defaultValue, ECVarType::Type cvartype, bool NeedsValue) :ConsoleVariable(name, cvartype, NeedsValue)
{
	CurrentValue.Int_Value = defaultValue;
	DefaultValue.Int_Value = defaultValue;
}
ConsoleVariable::ConsoleVariable(std::string name, float defaultValue, ECVarType::Type cvartype, bool NeedsValue) : ConsoleVariable(name, cvartype, NeedsValue)
{
	CurrentValue.F_Value = defaultValue;
	DefaultValue.F_Value = defaultValue;
	IsFloat = true;
}

ConsoleVariable::ConsoleVariable(std::string name, ECVarType::Type cvartype, bool NeedsValue)
{
	Type = cvartype;
	NeedsValue = NeedsValue;
	if (cvartype != ECVarType::LaunchOnly)
	{
		ConsoleVariableManager::Get()->ConsoleVars.push_back(this);
	}
	if (cvartype != ECVarType::ConsoleOnly)
	{
		ConsoleVariableManager::Get()->LaunchArgs.push_back(this);
	}
	ConsoleVariableManager::Get()->AllVars.push_back(this);
	Name = name;
	std::transform(Name.begin(), Name.end(), Name.begin(), ::tolower);
}

std::string ConsoleVariable::GetValueString()
{
	if (IsFloat)
	{
		return std::to_string(GetFloatValue());
	}
	return	std::to_string(GetIntValue());
}

bool GetValueClean(std::string value, int& outvalue)
{
	if (value.find('-') != -1)
	{
		outvalue = -1;
		return false;
	}
	outvalue = stoi(value);
	return true;
}

ConsoleVariableManager* ConsoleVariableManager::Get()
{
	if (Instance == nullptr)
	{
		Instance = new ConsoleVariableManager();
	}
	return Instance;
}

void ConsoleVariableManager::SetupCVarsFromCFG(std::vector<std::string>& VarLines)
{
	for (int i = 0; i < VarLines.size(); i++)
	{
		ProcessVarString(VarLines[i], false);
	}
}

void ConsoleVariableManager::ProcessVarString(std::string value, bool LaunchOnly)
{
	std::transform(value.begin(), value.end(), value.begin(), ::tolower);
	std::vector<std::string> SplitArgs = StringUtils::Split(value, ' ');
	for (ConsoleVariable* CV : Instance->AllVars)
	{
		if (LaunchOnly && (CV->Type == ECVarType::LaunchOnly || CV->Type == ECVarType::ConsoleAndLaunch))
		{
			continue;
		}
		for (int i = 0; i < SplitArgs.size(); i++)
		{
			std::string Arg = SplitArgs[i];
			if (Arg.compare(CV->GetName()) == 0)
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
		}
	}
}

void ConsoleVariableManager::GetCFGVariables(std::vector<std::string> &Lines)
{
	for (ConsoleVariable* CV : Instance->AllVars)
	{
		if (!CV->IsDefaultValue())
		{
			Lines.push_back(CV->GetName() + " " + std::to_string(CV->GetIntValue()));
		}
	}
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

bool ConsoleVariableManager::TrySetCVar(std::string command, ConsoleVariable** Var)
{
	if (command.length() == 0)
	{
		return false;
	}
	std::transform(command.begin(), command.end(), command.begin(), ::tolower);
	std::vector<std::string> SplitArgs = StringUtils::Split(command, ' ');
	if (SplitArgs.size() == 0)
	{
		return false;
	}
	for (ConsoleVariable* CV : Instance->ConsoleVars)
	{
		if (CV->GetName() == SplitArgs[0])
		{
			*Var = CV;
			if (SplitArgs.size() > 1)
			{
				if (CV->IsFloat)
				{
					float value = stof(SplitArgs[1]);
					CV->SetValueF(value);
				}
				else
				{
					int value = stoi(SplitArgs[1]);
					CV->SetValue(value);
				}

				return true;
			}
			return true;
		}
	}
	return false;
}

void IConsoleSettings::GetVariables(std::vector<ConsoleVariable*>& VarArray)
{
	Seralise();
	for (auto itor = VarMap.begin(); itor != VarMap.end(); itor++)
	{
		itor->second.Cvar = new ConsoleVariable(itor->first, 0, ECVarType::ConsoleAndLaunch, false);
		VarArray.push_back(itor->second.Cvar);
	}
}

void IConsoleSettings::GatherData()
{
	IsReading = true;
	Seralise();
}

void IConsoleSettings::LinkProp(std::string name, int* value)
{
	if (IsReading)
	{
		IConsoleSettingsVar* Var = nullptr;
		auto itor = VarMap.find(name);
		if (itor != VarMap.end())
		{
			Var = &itor->second;
		}
		if (Var != nullptr && Var->Cvar != nullptr)
		{
			*value = Var->Cvar->GetIntValue();
		}
	}
	else
	{
		VarMap.emplace(name, IConsoleSettingsVar(value));
	}
}
