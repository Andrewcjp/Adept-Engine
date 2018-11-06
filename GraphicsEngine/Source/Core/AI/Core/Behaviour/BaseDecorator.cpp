
#include "BaseDecorator.h"
#include "AI/Core/Behaviour/BTBlackboard.h"
#include "Core/Platform/PlatformCore.h"
#include "Core/Platform/Logger.h"

BaseDecorator::BaseDecorator(BTValue* value, EDecoratorTestType::Type t)
{
	ConditonalValue = value;
	TestType = t;
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
	}
	ensure(false);
	return false;
}