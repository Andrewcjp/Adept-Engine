#pragma once
#include "RHI\RHIInterGPUStagingResource.h"
class D3D12InterGPUStagingResource : public RHIInterGPUStagingResource
{
public:
	D3D12InterGPUStagingResource(DeviceContext* owner, const InterGPUDesc& desc);
	~D3D12InterGPUStagingResource();
	ID3D12Resource* GetViewOnDevice(int index);
private:
	void Init();
	void CreateForGPU(int index);
	struct GPUViewOfStageResource
	{
		ID3D12Heap* SharedHeap = nullptr;
		ID3D12Resource* SharedResource = nullptr;
	};
	//the Owner device hold the main copy of the heap
	ID3D12Heap* MainHeap = nullptr;
	ID3D12Resource* MainResource = nullptr;
	GPUViewOfStageResource GPUViews[MAX_GPU_DEVICE_COUNT];
	D3D12_RESOURCE_DESC CrossAdapterDesc;
	HANDLE heapHandle = nullptr;
};

