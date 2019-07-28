#pragma once
#include "../Utils/TypeUtils.h"
#include <functional>


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
	const std::string& GetName()const;
	std::string GetLaunchName()const;

	CORE_API bool GetBoolValue() const;
	CORE_API void SetValue(int value);
	CORE_API void SetValueF(float value);
	CORE_API int GetIntValue() const;
	CORE_API bool IsValueVar() const;
	CORE_API float GetFloatValue() const;
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
	std::function<void(int state)> OnChangedFunction;
	std::function<void(bool state)> OnChangedBoolFunction;
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
	IConsoleSettingsVar()
	{}
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
	virtual ~IConsoleSettings();
protected:

	virtual void Seralise() = 0;

	void LinkProp(std::string name, int* value);
	void GatherData();
	std::map<std::string, IConsoleSettingsVar> VarMap;
	bool IsReading = false;
};