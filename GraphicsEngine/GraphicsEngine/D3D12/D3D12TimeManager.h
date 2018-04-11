#pragma once
#include "D3D12RHI.h"
class D3D12TimeManager
{
public:
	D3D12TimeManager();
	~D3D12TimeManager();
	void Initialize(uint32_t MaxNumTimers);

	void BeginReadBack(void);


private:
	uint32_t sm_GpuTickDelta = 0;
	ID3D12Resource* sm_ReadBackBuffer = nullptr;
	ID3D12Heap* sm_QueryHeap = nullptr;
	uint32_t sm_MaxNumTimers = 0;
};

