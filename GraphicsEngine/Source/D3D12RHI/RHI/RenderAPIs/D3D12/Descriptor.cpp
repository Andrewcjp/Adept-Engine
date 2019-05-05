#include "D3D12RHIPCH.h"
#include "Descriptor.h"
#include "DescriptorHeap.h"


Descriptor::Descriptor()
{}


Descriptor::~Descriptor()
{}

void Descriptor::Init(D3D12_DESCRIPTOR_HEAP_TYPE T, DescriptorHeap* heap, int size)
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

void Descriptor::Recreate()
{
	if (TargetResource != nullptr)
	{
		if (DescriptorType == EDescriptorType::SRV)
		{
			Owner->GetDevice()->GetDevice()->CreateShaderResourceView(TargetResource, &SRVDesc, GetCPUAddress(OffsetInHeap));
		}
		else if (DescriptorType == EDescriptorType::UAV)
		{
			Owner->GetDevice()->GetDevice()->CreateUnorderedAccessView(TargetResource, UAVCounterResource, &UAVDesc, GetCPUAddress(OffsetInHeap));
		}
		else if (DescriptorType == EDescriptorType::CBV)
		{

		}
	}
}

void Descriptor::CreateShaderResourceView(ID3D12Resource * pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC * pDesc, int offset)
{
	DescriptorType = EDescriptorType::SRV;
	SRVDesc = *pDesc;
	TargetResource = pResource;
	OffsetInHeap = offset;
	Recreate();
}

void Descriptor::CreateUnorderedAccessView(ID3D12Resource * pResource, ID3D12Resource * pCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC * pDesc, int offset)
{
	DescriptorType = EDescriptorType::UAV;
	UAVDesc = *pDesc;
	TargetResource = pResource;
	UAVCounterResource = pCounterResource;
	OffsetInHeap = offset;
	Recreate();
}

void Descriptor::Release()
{
	Owner->RemoveDescriptor(this);
}

