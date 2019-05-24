#pragma once
#include "D3D12RHI.h"
class DescriptorHeap;
class Descriptor;
class D3D12DeviceContext;
//points to 2 or more descriptors one in flight and one in record.
//handles updating when in flight switches with record.
class DescriptorGroup
{
public:
	DescriptorGroup();
	~DescriptorGroup();
	void Init(D3D12_DESCRIPTOR_HEAP_TYPE Type, DescriptorHeap* heap, int size);

	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUAddress(int index = 0);
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUAddress(int index = 0);
	int indexInHeap = 0;
	D3D12_DESCRIPTOR_HEAP_TYPE GetType();
	int GetSize();
	DescriptorHeap* Owner;
	void Recreate();
	void CreateShaderResourceView(ID3D12Resource  *pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC *pDesc, int offset = 0);
	void CreateUnorderedAccessView(ID3D12Resource *pResource, ID3D12Resource *pCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC *pDesc, int offset = 0);
	void OnFrameSwitch();
	Descriptor* GetDescriptor(int index);
	void Release();
private:
	Descriptor* Descriptors[RHI::CPUFrameCount];
	D3D12DeviceContext* context = nullptr;
	bool RecreateQueued = false;
};

