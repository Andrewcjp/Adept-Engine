#pragma once

#include "UIInspectorBase.h"
#if WITH_EDITOR
class IEdtiorInspectable
{
public:
	IEdtiorInspectable();
	virtual ~IEdtiorInspectable();

	//called when this item is instpected
	virtual void OnInspected();
	//called when desleected
	virtual void OnDeselected();
	//used to populate the feilds in this object.
	virtual std::vector<InspectorProperyGroup> GetInspectorFields() = 0;

};
#endif
