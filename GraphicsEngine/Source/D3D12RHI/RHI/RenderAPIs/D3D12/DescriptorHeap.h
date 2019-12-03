#pragma once
#include "RHI\RHITypes.h"
class DeviceContext;
class D3D12CommandList;
class DXDescriptor;
class D3D12DeviceContext;
class DescriptorHeap : public IRHIResourse
{
public:
	enum EGPUMemoryPriority
	{
		Critical,
		Scaled,
		RenderBuffers,
		Streaming
	};
	DescriptorHeap(DescriptorHeap* other, int newsize);
	DescriptorHeap(DeviceContext* inDevice, int Num, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);

	void CreateHeap();

	~DescriptorHeap();
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuAddress(int index);
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUAddress(int index);
	void SetName(LPCWSTR name);
	void BindHeap(D3D12CommandList * list);
	void Release() override;
	void SetPriority(EGPUMemoryPriority NewPriority);
	ID3D12DescriptorHeap* GetHeap();
	std::string GetDebugName();
	void AddDescriptor(DXDescriptor* desc, bool Create = true);
	int GetNumberOfDescriptors();
	int GetMaxSize();
	int GetNextFreeIndex();
	void MoveAllToHeap(DescriptorHeap* heap, int offset = 0);
	D3D12DeviceContext* GetDevice();
	void RemoveDescriptor(DXDescriptor* desc);
	void ClearHeap();
	DXDescriptor* CopyToHeap(DXDescriptor* desc);
	DXDescriptor* AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, int size = 1);
private:
	std::vector<DXDescriptor*> ContainedDescriptors;
	ID3D12DescriptorHeap * mHeap = nullptr;
	D3D12DeviceContext* Device = nullptr;
	uint32 DescriptorOffsetSize = 0;
	EGPUMemoryPriority Priority = EGPUMemoryPriority::Critical;
	D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
	uint32 DescriptorCount = 0;
};

