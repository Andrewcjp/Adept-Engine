#include "Stdafx.h"
#include "BTBlackboard.h"


BTValue * BTBlackboard::AddValue(EBTBBValueType::Type type)
{
	BTValue* newvalue = new BTValue();
	newvalue->ValueType = type;//todfo: name and inital value
	return newvalue;
}

BTBlackboard::BTBlackboard()
{}


BTBlackboard::~BTBlackboard()
{}
