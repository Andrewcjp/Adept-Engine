#pragma once
#include "RHI\RHITypes.h"
class DeviceContext;
class D3D12CommandList;
class DXDescriptor;
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
	~DescriptorHeap();
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuAddress(int index);
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUAddress(int index);
	void SetName(LPCWSTR name);
	void BindHeap(D3D12CommandList * list);
	void Release() override;
	void SetPriority(EGPUMemoryPriority NewPriority);
	ID3D12DescriptorHeap* GetHeap();;
	std::string GetDebugName();
	void BindHeap_Old(ID3D12GraphicsCommandList* list);
	void AddDescriptor(DXDescriptor* desc);
	int GetNumberOfDescriptors();
	int GetMaxSize();
	int GetNextFreeIndex();
	void MoveAllToHeap(DescriptorHeap* heap, int offset = 0);
	D3D12DeviceContext* GetDevice();
	void RemoveDescriptor(DXDescriptor* desc);
private:
	std::vector<DXDescriptor*> ContainedDescriptors;
	ID3D12DescriptorHeap * mHeap = nullptr;
	class D3D12DeviceContext* Device = nullptr;
	int DescriptorOffsetSize = 0;
	EGPUMemoryPriority Priority = EGPUMemoryPriority::Critical;
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	int DescriptorCount = 0;
};

