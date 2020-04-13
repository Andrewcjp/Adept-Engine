#pragma once
#include "RHI\RHITypes.h"
class DeviceContext;
class D3D12CommandList;
class DXDescriptor;
class D3D12DeviceContext;
class DescriptorHeap : public IRHIResourse
{
public:
	DescriptorHeap(DescriptorHeap* other, int newsize);
	DescriptorHeap(DeviceContext* inDevice, int Num, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	~DescriptorHeap();

	void CreateHeap();
	void SetName(LPCWSTR name);
	void Release() override;
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUAddress(uint index);
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUAddress(uint index);
	void BindHeap(D3D12CommandList * list);
	ID3D12DescriptorHeap* GetHeap();
	std::string GetDebugName();
	void AddDescriptor(DXDescriptor* desc, bool Create = true);
	void RemoveDescriptor(DXDescriptor* desc);
	void ClearHeap();

	uint64 GetNumberOfDescriptors();
	uint64 GetNumberOfDescriptorsForStats();
	UINT GetMaxSize();
	uint GetNextFreeIndex();

	D3D12DeviceContext* GetDevice();
	DXDescriptor* CopyToHeap(DXDescriptor* desc);
	void CopyToHeap(DescriptorHeap* heap);
	DXDescriptor* AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, int size = 1);
	void SetFrameGuardBound();
private:
	std::vector<DXDescriptor*> ContainedDescriptors;
	ID3D12DescriptorHeap * mHeap = nullptr;
	D3D12DeviceContext* Device = nullptr;
	uint32 DescriptorOffsetSize = 0;
	D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
	uint32 DescriptorCount = 0;
	//Offset indicating the point at with CPU frame 1 starts.
	uint64 FrameBoundEnd = 0;
	uint NextFreeIndex = 0;
	uint CachedSize = 0;
};

