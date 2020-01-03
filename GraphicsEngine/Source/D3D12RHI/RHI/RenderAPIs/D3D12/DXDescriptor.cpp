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
	ItemDesc.DescriptorCount = size;
}

D3D12_GPU_DESCRIPTOR_HANDLE DXDescriptor::GetGPUAddress(int index)
{
	return Owner->GetGPUAddress(indexInHeap + index);
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
	return ItemDesc.DescriptorCount;
}

void DXDescriptor::Recreate()
{
	for (int i = 0; i < ItemDesc.DescriptorCount; i++)
	{
		DescData* Desc = &ItemDesc.Data[i];
		if (Desc->NeedsUpdate)
		{
			if (ItemDesc.DescriptorType == EDescriptorType::SRV)
			{
				Desc->NeedsUpdate = false;
				if (Desc->TargetResource != nullptr)
				{
					Desc->TargetResource->AddRef();
					Desc->TargetResource->Release();
				}
				Owner->GetDevice()->GetDevice()->CreateShaderResourceView(Desc->TargetResource, &Desc->SRVDesc, GetCPUAddress(Desc->OffsetInHeap));
			}
			else if (ItemDesc.DescriptorType == EDescriptorType::UAV)
			{
				Desc->NeedsUpdate = false;
				Owner->GetDevice()->GetDevice()->CreateUnorderedAccessView(Desc->TargetResource, Desc->UAVCounterResource, &Desc->UAVDesc, GetCPUAddress(Desc->OffsetInHeap));
			}
			else if (ItemDesc.DescriptorType == EDescriptorType::CBV)
			{
				//unused for now
			}
		}
	}
}

void DXDescriptor::CreateUnorderedAccessView(ID3D12Resource * pResource, ID3D12Resource * pCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC * pDesc, int offset)
{
	ItemDesc.CreateUnorderedAccessView(pResource, pCounterResource, pDesc, offset);
}

void DXDescriptor::CreateShaderResourceView(ID3D12Resource * pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC * pDesc, int offset)
{
	ItemDesc.CreateShaderResourceView(pResource, pDesc, offset);
}

void DescriptorItemDesc::CreateShaderResourceView(ID3D12Resource * pResource, const D3D12_SHADER_RESOURCE_VIEW_DESC * pDesc, int offset)
{
	DescriptorType = EDescriptorType::SRV;
	if (Data.size() <= offset)
	{
		Data.resize(offset+1);
	}
	DescData* Desc = &Data[offset];
	Desc->SRVDesc = *pDesc;
	Desc->TargetResource = pResource;
	Desc->OffsetInHeap = offset;
	Desc->NeedsUpdate = true;
}

void DescriptorItemDesc::CreateUnorderedAccessView(ID3D12Resource * pResource, ID3D12Resource * pCounterResource, const D3D12_UNORDERED_ACCESS_VIEW_DESC * pDesc, int offset)
{
	DescriptorType = EDescriptorType::UAV;
	if (Data.size() <= offset)
	{
		Data.resize(offset + 1);
	}
	DescData* Desc = &Data[offset];
	Desc->UAVDesc = *pDesc;
	Desc->TargetResource = pResource;
	Desc->UAVCounterResource = pCounterResource;
	Desc->OffsetInHeap = offset;
	Desc->NeedsUpdate = true;
	if (Desc->UAVDesc.ViewDimension == D3D12_UAV_DIMENSION_TEXTURE3D)
	{
		Desc->UAVDesc.Texture3D.WSize = -1;
	}
}

uint64 DescriptorItemDesc::GetHash()const 
{
	return DXDescriptor::GetItemDescHash(*this);
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
	return ItemDesc.DescriptorType != EDescriptorType::Limit;
}

void DXDescriptor::InitFromDesc(DXDescriptor * other)
{
	ItemDesc.DescriptorType = other->ItemDesc.DescriptorType;
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
	return GetItemDescHash(ItemDesc);
}

uint64 DXDescriptor::GetItemDescHash(const DescriptorItemDesc& descdesc)
{
	uint64 hash = 0;
	HashUtils::hash_combine(hash, descdesc.DescriptorCount);
	for (int i = 0; i < descdesc.DescriptorCount; i++)
	{
		HashUtils::hash_combine(hash, descdesc.Data[i].TargetResource);
		if (descdesc.DescriptorType == EDescriptorType::SRV)
		{
			HashUtils::hash_combine(hash, GetSRVHash(descdesc.Data[i].SRVDesc));
		}
		else if (descdesc.DescriptorType == EDescriptorType::UAV)
		{
			HashUtils::hash_combine(hash, descdesc.Data[i].UAVCounterResource);
			HashUtils::hash_combine(hash, GetUAVHash(descdesc.Data[i].UAVDesc));
		}
	}
	return hash;
}

void DXDescriptor::SetItemDesc(DescriptorItemDesc itemdesc)
{
	if (ItemDesc.DescriptorCount != itemdesc.DescriptorCount)
	{
		itemdesc.DescriptorCount = ItemDesc.DescriptorCount;
	}
	ItemDesc = itemdesc;
}

