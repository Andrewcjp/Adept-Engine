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
		SavedOnly, //< not sure what is quite is in the category but okay.
		Limit
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
		return CurrentValue.Int_Value;
	}
	void SetValue(int value)
	{
		CurrentValue.Int_Value = value;
	}
	void SetValueF(float value)
	{
		CurrentValue.F_Value = value;
	}
	int GetIntValue() const
	{
		return CurrentValue.Int_Value;
	}
	bool IsValueVar() const
	{
		return NeedsValue;
	}
	float GetFloatValue() const
	{
		return CurrentValue.F_Value;
	}
	template<class T>
	T GetAsEnum()
	{
		return TypeUtils::GetFromInt<T>(CurrentValue.Int_Value);
	}
	bool IsDefaultValue()const
	{
		if (IsFloat)
		{
			return CurrentValue.F_Value == DefaultValue.F_Value;
		}
		return CurrentValue.Int_Value == DefaultValue.Int_Value;
	}

	std::string GetValueString();
	bool IsFloat = false;
	ECVarType::Type Type;
private:
	ConsoleVariable(std::string name, ECVarType::Type cvartype, bool NeedsValue);
	std::string Name = "";
	bool NeedsValue = false;
	union ValueUnion
	{
		int Int_Value;
		float F_Value;
	};
	ValueUnion CurrentValue;
	ValueUnion DefaultValue;
};

class ConsoleVariableManager
{
public:
	static ConsoleVariableManager * Instance;
	std::vector<ConsoleVariable*> ConsoleVars;
	std::vector<ConsoleVariable*> LaunchArgs;
	std::vector<ConsoleVariable*> AllVars;
	static ConsoleVariableManager* Get();
	void SetupCVarsFromCFG(std::vector<std::string> & VarLines);
	void ProcessVarString(std::string value, bool LaunchOnly);
	void GetCFGVariables(std::vector<std::string>& Lines);
	static void SetupVars(std::string LaunchArgString);
	static bool TrySetCVar(std::string command, ConsoleVariable ** Var);
};
struct IConsoleSettingsVar
{
	IConsoleSettingsVar(){}
	IConsoleSettingsVar(int* value)
	{
		Propptr = value;
	}
	IConsoleSettingsVar(float* value)
	{
		Propptr = value;
		IsFloat = true;
	}
	void* Propptr = nullptr;
	ConsoleVariable* Cvar = nullptr;
	bool IsFloat = false;
};
class IConsoleSettings
{
public:
	void GetVariables(std::vector<ConsoleVariable*> & VarArray);

protected:

	virtual void Seralise() = 0;

	void LinkProp(std::string name, int* value);
	void GatherData();
	std::map<std::string, IConsoleSettingsVar> VarMap;
	bool IsReading = false;
};