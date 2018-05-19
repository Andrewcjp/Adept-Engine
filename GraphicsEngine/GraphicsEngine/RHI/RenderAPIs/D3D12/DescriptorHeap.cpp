#include "stdafx.h"
#include "DescriptorHeap.h"
#include <algorithm>

DescriptorHeap::DescriptorHeap(DeviceContext* inDevice,int Num, D3D12_DESCRIPTOR_HEAP_TYPE type, D3D12_DESCRIPTOR_HEAP_FLAGS flags)
{
	Device = inDevice;
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = std::max(Num, 1);
	srvHeapDesc.Type = type;
	srvHeapDesc.Flags = flags;
	ThrowIfFailed(Device->GetDevice()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mHeap)));
	DescriptorOffsetSize = Device->GetDevice()->GetDescriptorHandleIncrementSize(type);
}

DescriptorHeap::~DescriptorHeap()
{
	mHeap->Release();
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGpuAddress(int index)
{
	return 	CD3DX12_GPU_DESCRIPTOR_HANDLE(mHeap->GetGPUDescriptorHandleForHeapStart(), index, DescriptorOffsetSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCPUAddress(int index)
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(mHeap->GetCPUDescriptorHandleForHeapStart(), index, DescriptorOffsetSize);
}

void DescriptorHeap::SetName(LPCWSTR name)
{
	mHeap->SetName(name);
}

void DescriptorHeap::BindHeap(ID3D12GraphicsCommandList * list)
{
	ID3D12DescriptorHeap* ppHeaps[] = { mHeap };
	list->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);
}
