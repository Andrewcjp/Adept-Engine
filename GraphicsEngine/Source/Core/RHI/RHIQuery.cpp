#include "RHIQuery.h"


RHIQuery::RHIQuery(EGPUQueryType::Type Type, DeviceContext * device)
{
	Device = device;
	QType = Type;
}

RHIQuery::~RHIQuery()
{}

EGPUQueryType::Type RHIQuery::GetQueryType() const
{
	return QType;
}


