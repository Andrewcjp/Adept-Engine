#include "stdafx.h"
#include "D3D12Heap.h"


D3D12Heap::D3D12Heap()
{}


D3D12Heap::~D3D12Heap()
{
	
}

void D3D12Heap::CreateStaticHeap(int NumDescriptors, D3D12Heap::HeapType type)
{
	IsDynamic = false;
	CurrentType = type;
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = NumDescriptors;
	ensure(D3D12_DESCRIPTOR_HEAP_TYPE_RTV == HeapType::RTV);
	srvHeapDesc.Type = (D3D12_DESCRIPTOR_HEAP_TYPE)type;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(D3D12RHI::GetDevice()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_Heap)));
}

void D3D12Heap::CreateDynamicHeap(int NumDescriptors, HeapType type)
{
	IsDynamic = true;
}
