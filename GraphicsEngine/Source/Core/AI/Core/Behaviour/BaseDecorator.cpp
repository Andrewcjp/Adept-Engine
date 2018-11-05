#include "Stdafx.h"
#include "BaseDecorator.h"
#include "AI/Core/Behaviour/BTBlackboard.h"


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
		break;
	case EDecoratorTestType::NotNull:
		return ConditonalValue->IsValid();
		break;
	case EDecoratorTestType::NotZero:
		float t = *ConditonalValue->GetValue<float>();
		return t != 0.0f;
		break;
	}
	return false;
}
