#pragma once
#include <vector>
#include "Inspector.h"
class IInspectable
{
public:
	IInspectable();
	~IInspectable();
	//called whne this item is instpected
	virtual void OnInspected();
	//called when desleected
	virtual void OnDeselected();
	//used to populate the feilds in this object.
	virtual std::vector<Inspector::InspectorProperyGroup> GetInspectorFields() = 0;
};

