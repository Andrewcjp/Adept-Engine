#include "D3D12RHIPCH.h"
#include "GPUMemoryPage.h"
#include "Core/Utils/StringUtil.h"
#include "D3D12Helpers.h"
#include "GPUResource.h"
#include "D3D12DeviceContext.h"
static ConsoleVariable LogPageAllocations("VMEM.LogAlloc", 0, ECVarType::ConsoleAndLaunch);
GPUMemoryPage::GPUMemoryPage(AllocDesc & desc, D3D12DeviceContext* context)
{
	PageDesc = desc;
	Device = context;
	InitHeap();
}

GPUMemoryPage::~GPUMemoryPage()
{
	IsReleaseing = true;
	MemoryUtils::DeleteReleaseableVector(ContainedResources);
	SafeRelease(PageHeap);
}

EAllocateResult::Type GPUMemoryPage::Allocate(AllocDesc & desc, GPUResource** Resource)
{
	CalculateSpaceNeeded(desc);
	if (!CheckSpaceForResource(desc))
	{
		return EAllocateResult::NoSpace;
	}


	//DXMM: Todo Placed resource 
	ID3D12Resource* DxResource = nullptr;
	CreateResource(GetChunk(desc), desc, &DxResource);
	if (desc.Name.length() > 0)
	{
		std::wstring Conv = StringUtils::ConvertStringToWide(desc.Name);
		DxResource->SetName(Conv.c_str());
	}
	*Resource = new GPUResource(DxResource, desc.InitalState, Device);
	(*Resource)->SetDebugName(desc.Name);
	ContainedResources.push_back(*Resource);
	(*Resource)->SetGPUPage(this);
	if (LogPageAllocations.GetBoolValue())
	{
		//Log::LogMessage("Allocating " + std::to_string(desc.Size / 1e6) + "MB Called '" + desc.Name + "' in Segment " + EGPUMemorysegment::ToString(desc.Segment));
		Log::LogMessage("Page '" + PageDesc.Name + "' Used " + StringUtils::ByteToMB(OffsetInPlacedHeap) + " / " + StringUtils::ByteToMB(PageDesc.Size));
	}
	return EAllocateResult::OK;
}

void GPUMemoryPage::CalculateSpaceNeeded(AllocDesc & desc)
{
	desc.TextureAllocData = Device->GetDevice()->GetResourceAllocationInfo(0, 1, &desc.ResourceDesc);
	desc.ResourceDesc.Alignment = desc.TextureAllocData.Alignment;
}

bool GPUMemoryPage::CheckSpaceForResource(AllocDesc & desc)
{
	if (FindFreeChunk(desc) == nullptr)
	{
		return false;
	}
	return true;
}

GPUMemoryPage::AllocationChunk * GPUMemoryPage::FindFreeChunk(AllocDesc & desc)
{
	for (int i = 0; i < FreeChunks.size(); i++)
	{
		if (FreeChunks[i]->CanFitAllocation(desc))
		{
			return FreeChunks[i];
		}
	}
	return nullptr;
}

GPUMemoryPage::AllocationChunk * GPUMemoryPage::AllocateFromFreeChunk(AllocDesc & desc)
{
	AllocationChunk* Source = FindFreeChunk(desc);
	if (Source == nullptr)
	{
		return nullptr;
	}
	AllocationChunk* NewChunk = new AllocationChunk();
	const uint64 ResourceSize = D3D12Helpers::Align(desc.TextureAllocData.SizeInBytes);

	NewChunk->offset = Source->offset;
	Source->offset += ResourceSize;

	NewChunk->size = ResourceSize;
	AllocatedChunks.push_back(NewChunk);
	if (Source->size == ResourceSize)
	{
		SafeDelete(Source);
		VectorUtils::Remove(FreeChunks, Source);
	}
	else
	{
		Source->size -= ResourceSize;
	}
	return NewChunk;
}

GPUMemoryPage::AllocationChunk * GPUMemoryPage::GetChunk(AllocDesc & desc)
{
	return AllocateFromFreeChunk(desc);
}

bool GPUMemoryPage::AllocationChunk::CanFitAllocation(const AllocDesc & desc) const
{
	return size > D3D12Helpers::Align(desc.TextureAllocData.SizeInBytes);
}

void GPUMemoryPage::CreateResource(AllocationChunk* chunk, AllocDesc & desc, ID3D12Resource** Resource)
{
	D3D12_CLEAR_VALUE* value = nullptr;
	//if (desc.ResourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER && PageDesc.PageAllocationType == EPageTypes::RTAndDS_Only)
	{
		if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET || desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
		{
			value = &desc.ClearValue;
		}
	}
	ThrowIfFailed(Device->GetDevice()->CreatePlacedResource(PageHeap, chunk->offset, &desc.ResourceDesc, desc.InitalState, value, IID_PPV_ARGS(Resource)));
}

void GPUMemoryPage::InitHeap()
{
	D3D12_HEAP_DESC desc = {};
	{
		// To avoid wasting memory SizeInBytes should be 
		// multiples of the effective alignment [Microsoft 2018a]
		desc.SizeInBytes = PageDesc.Size;
		desc.Alignment = PageDesc.Alignment;
		if (PageDesc.PageAllocationType == EPageTypes::BufferUploadOnly)
		{
			desc.Properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		}
		else
		{
			desc.Properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		}
		desc.Flags = GetFlagsForType(PageDesc.PageAllocationType);

		ThrowIfFailed(Device->GetDevice()->CreateHeap(&desc, IID_PPV_ARGS(&PageHeap)));
	}
	AllocationChunk* chunk = new AllocationChunk();
	chunk->size = PageDesc.Size;
	FreeChunks.push_back(chunk);
}

D3D12_HEAP_FLAGS GPUMemoryPage::GetFlagsForType(EPageTypes::Type T)
{
	switch (T)
	{
		case EPageTypes::All:
			return D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES;
		case EPageTypes::RTAndDS_Only:
			return D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES;
		case EPageTypes::BuffersOnly:
		case EPageTypes::BufferUploadOnly:
			return D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
		case EPageTypes::TexturesOnly:
			return D3D12_HEAP_FLAG_NONE | D3D12_HEAP_FLAG_DENY_BUFFERS | D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES;
	}
	return D3D12_HEAP_FLAG_NONE;
}

void GPUMemoryPage::Compact()
{
	//#DXMM: Defragment the page 


}

void GPUMemoryPage::Deallocate(GPUResource * R)
{
	if (IsReleaseing)
	{
		return;//during the delete we destroy any present GPU resources, we don't need them telling us there gone again.
	}
	VectorUtils::Remove(ContainedResources, R);
	if (ContainedResources.size() == 0)
	{
		OffsetInPlacedHeap = 0;
	}
}

UINT64 GPUMemoryPage::GetSize() const
{
	return PageDesc.Size;
}

UINT64 GPUMemoryPage::GetSizeInUse() const
{
	UINT64 Bytes = 0;
	for (int i = 0; i < AllocatedChunks.size(); i++)
	{
		Bytes += AllocatedChunks[i]->size;
	}
	return Bytes;
}

void GPUMemoryPage::LogReport()
{
	Log::LogMessage("Page '" + PageDesc.Name + "' Has " + std::to_string(ContainedResources.size()) + " resources using " + StringUtils::ByteToMB(GetSizeInUse()) + " / " 
		+ StringUtils::ByteToMB(PageDesc.Size) + " Free Chunks " + std::to_string(FreeChunks.size()));
}

void GPUMemoryPage::ResetPage()
{
	for (GPUResource* r : ContainedResources)
	{
		r->Release();
	}
	ContainedResources.clear();
	AllocatedChunks.clear();
	FreeChunks.clear();
	AllocationChunk* chunk = new AllocationChunk();
	chunk->size = PageDesc.Size;
	FreeChunks.push_back(chunk);
}

void GPUMemoryPage::EvictPage()
{
	IsResident = false;
	ID3D12Pageable* list = { PageHeap };
	Device->GetDevice()->Evict(1, &list);
}

void GPUMemoryPage::MakeResident()
{
	IsResident = true;
	ID3D12Pageable* list = { PageHeap };
	Device->GetDevice()->MakeResident(1, &list);
}

const AllocDesc & GPUMemoryPage::GetDesc() const
{
	return PageDesc;
}



