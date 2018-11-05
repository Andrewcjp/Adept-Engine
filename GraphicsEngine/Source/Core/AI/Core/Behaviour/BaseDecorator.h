#pragma once
struct BTValue;
namespace EDecoratorTestType
{
	enum Type
	{
		Null,
		NotNull,
		NotZero,
		Zero,
		Limit
	};
}
//Decorators Check the status of a value on the blackboard.
//they interface with selectors
class BaseDecorator
{
public:
	CORE_API BaseDecorator(BTValue* value, EDecoratorTestType::Type t);
	~BaseDecorator();
	virtual bool RunCheck();
	std::string Name = "";
private:
	BTValue* ConditonalValue = nullptr;
	EDecoratorTestType::Type TestType = EDecoratorTestType::Limit;
	
};

