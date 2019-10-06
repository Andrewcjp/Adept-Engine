#include "RHIStateObject.h"


RHIStateObject::RHIStateObject(DeviceContext* D,RHIStateObjectDesc desc)
{
	Device = D;
	Desc = desc;
}


RHIStateObject::~RHIStateObject()
{}

