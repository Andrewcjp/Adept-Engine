#pragma once
#include "../Utils/TypeUtils.h"


//Console vars can also be set though the launch arguments
namespace ECVarType
{
	enum Type
	{
		ConsoleOnly,
		LaunchOnly,
		ConsoleAndLaunch,
	};
}

class ConsoleVariable
{
public:
	CORE_API ConsoleVariable(std::string name, int DefaultValue, ECVarType::Type cvartype = ECVarType::ConsoleOnly, bool NeedsValue = false);
	CORE_API ConsoleVariable(std::string name, float DefaultValue, ECVarType::Type cvartype = ECVarType::ConsoleOnly, bool NeedsValue = false);
	~ConsoleVariable()
	{}
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
	void SetValueF(float value)
	{
		FloatValue = value;
	}
	int GetIntValue()
	{
		return CurrentValue;
	}
	bool IsValueVar()
	{
		return NeedsValue;
	}
	float GetFloatValue()
	{
		return FloatValue;
	}
	template<class T>
	T GetAsEnum()
	{
		return TypeUtils::GetFromInt<T>(CurrentValue);
	}
	std::string GetValueString();
	bool IsFloat = false;
private:
	std::string Name = "";
	int CurrentValue = 0;
	float FloatValue = 0.0f;
	bool NeedsValue = false;
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
	static void SetupVars(std::string LaunchArgString);
	static bool TrySetCVar(std::string command, ConsoleVariable ** Var);
};