#include "D3D12RHIPCH.h"
#include "Descriptor.h"
#include "DescriptorHeap.h"


Descriptor::Descriptor()
{}


Descriptor::~Descriptor()
{}

void Descriptor::Init(D3D12_DESCRIPTOR_HEAP_TYPE T, DescriptorHeap* heap,int size)
{
	Type = T;
	Owner = heap;
	DescriptorCount = size;
}

D3D12_GPU_DESCRIPTOR_HANDLE Descriptor::GetGPUAddress(int index)
{
	return Owner->GetGpuAddress(indexInHeap + index);
}

D3D12_CPU_DESCRIPTOR_HANDLE Descriptor::GetCPUAddress(int index)
{
	return Owner->GetCPUAddress(indexInHeap + index);
}

D3D12_DESCRIPTOR_HEAP_TYPE Descriptor::GetType()
{
	return Type;
}

int Descriptor::GetSize()
{
	return DescriptorCount;
}

