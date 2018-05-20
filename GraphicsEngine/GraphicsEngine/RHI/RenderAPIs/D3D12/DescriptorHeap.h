#pragma once
#include <d3d12.h>
#include "../RHI/DeviceContext.h"
class DescriptorHeap
{
public:
	DescriptorHeap::DescriptorHeap(DeviceContext* inDevice, int Num, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
	~DescriptorHeap();
	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuAddress(int index);
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUAddress(int index);
	void SetName(LPCWSTR name);
	void BindHeap(ID3D12GraphicsCommandList* list);
	void Release();
private:
	ID3D12DescriptorHeap * mHeap = nullptr;
	DeviceContext* Device = nullptr;
	int DescriptorOffsetSize = 0;
};

