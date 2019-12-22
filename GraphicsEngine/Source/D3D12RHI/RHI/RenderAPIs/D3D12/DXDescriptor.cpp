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
	ItemDesc.Data.resize(size);
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
		DescData* Desc = &ItemDesc.Data[i];
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
	DescData* Desc = &ItemDesc.Data[offset];
	Desc->SRVDesc = *pDesc;
	Desc->TargetResource = pResource;
	Desc->OffsetInHeap = offset;
	Desc->NeedsUpdate = true;
	//Recreate();
}

void DXDescriptor::CreateUnorderedAccessView(ID3D12Resource * pResource, ID3D12Resource * pCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC * pDesc, int offset)
{
	DescriptorType = EDescriptorType::UAV;
	DescData* Desc = &ItemDesc.Data[offset];
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
	ItemDesc.Data[0].NeedsUpdate = true;
}

bool DXDescriptor::GetNeedsUpdate()
{
	return ItemDesc.Data[0].NeedsUpdate;
}

bool DXDescriptor::IsTargetValid() const
{
	return ItemDesc.Data[0].TargetResource;
}

bool DXDescriptor::IsValid() const
{
	return DescriptorType != EDescriptorType::Limit;
}

void DXDescriptor::InitFromDesc(DXDescriptor * other)
{
	DescriptorType = other->DescriptorType;
	ItemDesc = ItemDesc;
}

uint64 GetSRVHash(const D3D12_SHADER_RESOURCE_VIEW_DESC& desc)
{
	uint64 hash = 0;
	HashUtils::hash_combine(hash, desc.Format);
	HashUtils::hash_combine(hash, desc.ViewDimension);
	HashUtils::hash_combine(hash, desc.Shader4ComponentMapping);
	//HashUtils::hash_combine(hash, desc.);
	return hash;
}

uint64 GetUAVHash(const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc)
{
	uint64 hash = 0;
	HashUtils::hash_combine(hash, desc.Format);
	HashUtils::hash_combine(hash, desc.ViewDimension);
	HashUtils::hash_combine(hash, desc.Texture2D.PlaneSlice);
	//HashUtils::hash_combine(hash, desc.);
	return hash;
}

uint64 DXDescriptor::GetHash() const
{
	uint64 hash = 0;
	HashUtils::hash_combine(hash, DescriptorCount);
	for (int i = 0; i < DescriptorCount; i++)
	{
		HashUtils::hash_combine(hash, ItemDesc.Data[i].TargetResource);
		if (DescriptorType == EDescriptorType::SRV)
		{
			HashUtils::hash_combine(hash, GetSRVHash(ItemDesc.Data[i].SRVDesc));
		}
		else if(DescriptorType == EDescriptorType::UAV)
		{
			HashUtils::hash_combine(hash, ItemDesc.Data[i].UAVCounterResource);
			HashUtils::hash_combine(hash, GetUAVHash(ItemDesc.Data[i].UAVDesc));
		}
	}
	return hash;
}

