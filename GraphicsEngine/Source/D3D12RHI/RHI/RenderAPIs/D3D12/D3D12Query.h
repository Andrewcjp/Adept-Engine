#pragma once
#include "RHI\RHIQuery.h"
class D3D12Query : public RHIQuery
{
public:
	D3D12Query(EGPUQueryType::Type Type, DeviceContext * device);
	virtual ~D3D12Query();
	int IndexInHeap = -1;
	UINT64 Result;
	D3D12_QUERY_DATA_PIPELINE_STATISTICS PipeStatResult;
	bool IsResolved = false;
	
private:
};

