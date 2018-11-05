#include "Stdafx.h"
#include "BTBlackboard.h"


BTValue * BTBlackboard::AddValue(EBTBBValueType::Type type)
{
	BTValue* newvalue = new BTValue();
	newvalue->ValueType = type;//todo: name and initial value
	return newvalue;
}

BTBlackboard::BTBlackboard()
{}


BTBlackboard::~BTBlackboard()
{}

bool BTValue::IsValid()
{
	switch (ValueType)
	{
	case EBTBBValueType::Vector:
		break;
	case EBTBBValueType::Float:
		break;
	case EBTBBValueType::Integer:
		break;
	case EBTBBValueType::Object:
		return ObjectPtr != nullptr;
		break;
	}
	return true;
}
