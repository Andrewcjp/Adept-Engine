#pragma once
/*!  \addtogroup AI
* @{ */
struct BTValue;
namespace EDecoratorTestType
{
	enum Type
	{
		Null,
		NotNull,
		NotZero,
		Zero,
		LessThanEqual,
		GreaterThanEqual,
		LessThan,
		GreaterThan,
		Limit
	};
}
//Decorators Check the status of a value on the blackboard.
//they interface with selectors
class BaseDecorator
{
public:
	CORE_API BaseDecorator(BTValue* value, EDecoratorTestType::Type t);
	CORE_API BaseDecorator(BTValue* value, EDecoratorTestType::Type t, float CheckValue);
	virtual ~BaseDecorator();
	virtual bool RunCheck();
	std::string Name = "";
private:
	BTValue* ConditonalValue = nullptr;
	EDecoratorTestType::Type TestType = EDecoratorTestType::Limit;
	float CheckValue = 0.0f;//only valid on Some variable types

};

