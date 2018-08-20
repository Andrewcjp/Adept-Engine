#pragma once
#include <algorithm>
//Console vars can also be set though the launch arguments
class ConsoleVariable
{
public:
	CORE_API ConsoleVariable(std::string name, int DefaultValue, bool AsLaunchArgs = false);
	~ConsoleVariable() {}
	const std::string& GetName()const
	{
		return Name;
	}
	std::string GetLaunchName()const
	{
		return "-" + Name;
	}

	bool GetBoolValue() const
	{
		return CurrentValue;
	}
	void SetValue(int value)
	{
		CurrentValue = value;
	}
private:
	std::string Name = "";
	int CurrentValue = 0;
};

class ConsoleVariableManager
{
public:
	static ConsoleVariableManager * Instance;
	std::vector<ConsoleVariable*> ConsoleVars;
	std::vector<ConsoleVariable*> LaunchArgs;
	static ConsoleVariableManager* Get()
	{
		if (Instance == nullptr)
		{
			Instance = new ConsoleVariableManager();
		}
		return Instance;
	}
	static void SetupCVars()
	{
		//todo: gather from config
	}
	static void SetupVars(std::string LaunchArgString)
	{
		std::transform(LaunchArgString.begin(), LaunchArgString.end(), LaunchArgString.begin(), ::tolower);		
		for (ConsoleVariable* CV : Instance->LaunchArgs)
		{
			if (LaunchArgString.compare(CV->GetLaunchName()) == 0)
			{
				CV->SetValue(1);
			}
		}
	}
};