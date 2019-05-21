#include "RHIQuery.h"


RHIQuery::RHIQuery(EGPUQueryType::Type Type, DeviceContext * device)
{
	Device = device;
}

RHIQuery::~RHIQuery()
{}

EGPUQueryType::Type RHIQuery::GetQueryType() const
{
	return QType;
}


