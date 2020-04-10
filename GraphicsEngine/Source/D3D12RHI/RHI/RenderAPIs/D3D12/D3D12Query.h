#pragma once
#include "RHI\RHIQuery.h"
class D3D12Query : public RHIQuery
{
public:
	D3D12Query(EGPUQueryType::Type Type, DeviceContext * device);
	virtual ~D3D12Query();
	int IndexInHeap = -1;
	UINT64 Result = 0;
	D3D12_QUERY_DATA_PIPELINE_STATISTICS PipeStatResult = D3D12_QUERY_DATA_PIPELINE_STATISTICS();
	bool IsResolved = false;
	
private:
};

