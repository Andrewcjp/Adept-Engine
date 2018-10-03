#pragma once
#include <vector>
#include "Inspector.h"
#if WITH_EDITOR
class IInspectable
{
public:
	IInspectable();
	~IInspectable();

	//called when this item is instpected
	virtual void OnInspected();
	//called when desleected
	virtual void OnDeselected();
	//used to populate the feilds in this object.
	virtual std::vector<Inspector::InspectorProperyGroup> GetInspectorFields() = 0;

};
#endif
