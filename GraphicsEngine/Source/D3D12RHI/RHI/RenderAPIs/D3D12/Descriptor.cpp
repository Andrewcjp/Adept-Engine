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
	Data.resize(size);
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
	for (int i = 0; i < DescriptorCount; i++)
	{
		DescData* Desc = &Data[i];
		if (Desc->NeedsUpdate)
		{
			if (DescriptorType == EDescriptorType::SRV)
			{
				Desc->NeedsUpdate = false;
				Owner->GetDevice()->GetDevice()->CreateShaderResourceView(Desc->TargetResource, &Desc->SRVDesc, GetCPUAddress(Desc->OffsetInHeap));
			}
			else if (DescriptorType == EDescriptorType::UAV)
			{
				Desc->NeedsUpdate = false;
				Owner->GetDevice()->GetDevice()->CreateUnorderedAccessView(Desc->TargetResource, Desc->UAVCounterResource, &Desc->UAVDesc, GetCPUAddress(Desc->OffsetInHeap));
			}
			else if (DescriptorType == EDescriptorType::CBV)
			{
				//unused for now
			}
		}
	}
}

void Descriptor::CreateShaderResourceView(ID3D12Resource * pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC * pDesc, int offset)
{
	DescriptorType = EDescriptorType::SRV;
	DescData* Desc = &Data[offset];
	Desc->SRVDesc = *pDesc;
	Desc->TargetResource = pResource;
	Desc->OffsetInHeap = offset;
	Desc->NeedsUpdate = true;
	//Recreate();
}

void Descriptor::CreateUnorderedAccessView(ID3D12Resource * pResource, ID3D12Resource * pCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC * pDesc, int offset)
{
	DescriptorType = EDescriptorType::UAV;
	DescData* Desc = &Data[offset];
	Desc->UAVDesc = *pDesc;
	Desc->TargetResource = pResource;
	Desc->UAVCounterResource = pCounterResource;
	Desc->OffsetInHeap = offset;
	Desc->NeedsUpdate = true;
	//Recreate();
}

void Descriptor::Release()
{
	Owner->RemoveDescriptor(this);
}

void Descriptor::SetOwner(DescriptorHeap * heap)
{
	Owner = heap;
	Data[0].NeedsUpdate = true;
}

