
#include "BaseDecorator.h"
#include "AI/Core/Behaviour/BTBlackboard.h"
#include "Core/Platform/PlatformCore.h"
#include "Core/Platform/Logger.h"

BaseDecorator::BaseDecorator(BTValue* value, EDecoratorTestType::Type t)
{
	ConditonalValue = value;
	TestType = t;
}

BaseDecorator::BaseDecorator(BTValue * value, EDecoratorTestType::Type t, float checkValue) :BaseDecorator(value, t)
{
	CheckValue = checkValue;
	if (value->ValueType == EBTBBValueType::Object)
	{
		Log::LogMessage("Object Type variables cannot be compared > or < ", Log::Warning);
	}
}

BaseDecorator::~BaseDecorator()
{}

bool BaseDecorator::RunCheck()
{
	switch (TestType)
	{
	case EDecoratorTestType::Null:
		return !ConditonalValue->IsValid();
	case EDecoratorTestType::NotNull:
		return ConditonalValue->IsValid();
	case EDecoratorTestType::NotZero:
		return !ConditonalValue->CheckZero();
	case EDecoratorTestType::Zero:
		return ConditonalValue->CheckZero();
	case EDecoratorTestType::GreaterThanEqual:
		return ConditonalValue->CheckGreater(CheckValue, true);
	case EDecoratorTestType::LessThanEqual:
		return ConditonalValue->CheckLess(CheckValue, true);
	case EDecoratorTestType::GreaterThan:
		return ConditonalValue->CheckGreater(CheckValue, false);
	case EDecoratorTestType::LessThan:
		return ConditonalValue->CheckLess(CheckValue, false);
	}
	ensure(false);
	return false;
}