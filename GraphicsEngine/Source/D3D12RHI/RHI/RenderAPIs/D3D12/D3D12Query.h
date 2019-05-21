#pragma once
#include "RHI\RHIQuery.h"
class D3D12Query : public RHIQuery
{
public:
	D3D12Query(EGPUQueryType::Type Type, DeviceContext * device);
	virtual ~D3D12Query();
	int IndexInHeap = 0;
	UINT64 Result;
	bool IsResolved = false;
private:
};

