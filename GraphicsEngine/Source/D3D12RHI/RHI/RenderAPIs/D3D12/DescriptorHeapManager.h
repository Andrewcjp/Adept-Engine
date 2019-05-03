#pragma once

class Descriptor;
class DescriptorHeap;
class D3D12CommandList;
class D3D12DeviceContext;
class DescriptorHeapManager
{
public:
	DescriptorHeapManager(D3D12DeviceContext* Device);
	void AllocateMainHeap(int size);
	~DescriptorHeapManager();
	Descriptor* AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type,int size = 1);
	DescriptorHeap* GetMainHeap();
	void BindHeap(D3D12CommandList* list);
private:
	D3D12DeviceContext* Device = nullptr;
	DescriptorHeap* MainHeap = nullptr;
	DescriptorHeap* SamplerHeap = nullptr;
};

