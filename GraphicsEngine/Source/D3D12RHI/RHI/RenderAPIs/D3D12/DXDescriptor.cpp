#include "DXDescriptor.h"
#include "DescriptorHeap.h"
#include "D3D12DeviceContext.h"



DXDescriptor::DXDescriptor()
{}


DXDescriptor::~DXDescriptor()
{}

void DXDescriptor::Init(D3D12_DESCRIPTOR_HEAP_TYPE T, DescriptorHeap* heap, int size)
{
	Type = T;
	Data.resize(size);
	Owner = heap;
	DescriptorCount = size;
}

D3D12_GPU_DESCRIPTOR_HANDLE DXDescriptor::GetGPUAddress(int index)
{
	return Owner->GetGpuAddress(indexInHeap + index);
}

D3D12_CPU_DESCRIPTOR_HANDLE DXDescriptor::GetCPUAddress(int index)
{
	return Owner->GetCPUAddress(indexInHeap + index);
}

D3D12_DESCRIPTOR_HEAP_TYPE DXDescriptor::GetType()
{
	return Type;
}

int DXDescriptor::GetSize()
{
	return DescriptorCount;
}

void DXDescriptor::Recreate()
{
	for (int i = 0; i < DescriptorCount; i++)
	{
		DescData* Desc = &Data[i];
		if (Desc->NeedsUpdate)
		{
			if (DescriptorType == EDescriptorType::SRV)
			{
				Desc->NeedsUpdate = false;
				if (Desc->TargetResource != nullptr)
				{
					Desc->TargetResource->AddRef();
					Desc->TargetResource->Release();
				}
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

void DXDescriptor::CreateShaderResourceView(ID3D12Resource * pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC * pDesc, int offset)
{
	DescriptorType = EDescriptorType::SRV;
	DescData* Desc = &Data[offset];
	Desc->SRVDesc = *pDesc;
	Desc->TargetResource = pResource;
	Desc->OffsetInHeap = offset;
	Desc->NeedsUpdate = true;
	//Recreate();
}

void DXDescriptor::CreateUnorderedAccessView(ID3D12Resource * pResource, ID3D12Resource * pCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC * pDesc, int offset)
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

void DXDescriptor::Release()
{
	Owner->RemoveDescriptor(this);
}

void DXDescriptor::SetOwner(DescriptorHeap * heap)
{
	Owner = heap;
	Data[0].NeedsUpdate = true;
}

bool DXDescriptor::NeedsUpdate()
{
	return Data[0].NeedsUpdate;
}

bool DXDescriptor::IsTargetValid() const
{
	return Data[0].TargetResource;
}

