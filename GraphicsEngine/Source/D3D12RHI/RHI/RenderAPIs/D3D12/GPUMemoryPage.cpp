#include "D3D12RHIPCH.h"
#include "GPUMemoryPage.h"
#include "Core/Utils/StringUtil.h"
#include "D3D12Helpers.h"
#include "GPUResource.h"
#include "D3D12DeviceContext.h"
static ConsoleVariable LogPageAllocations("VMEM.LogAlloc", 0, ECVarType::ConsoleAndLaunch);
GPUMemoryPage::GPUMemoryPage(AllocDesc & desc, D3D12DeviceContext* context)
{
	AddressAlignmentForce = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
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
	if (!CheckSpaceForResource(desc) && !desc.UseCommittedResource)
	{
		return EAllocateResult::NoSpace;
	}
	//DXMM: Todo reserved resources 
	ID3D12Resource* DxResource = nullptr;
	AllocationChunk* UsedChunk = GetChunk(desc);
	if (desc.UseCommittedResource)
	{
		CreateResourceCommitted(UsedChunk, desc, &DxResource);
	}
	else
	{
		CreateResource(UsedChunk, desc, &DxResource);
	}
	if (desc.Name.length() > 0)
	{
		std::wstring Conv = StringUtils::ConvertStringToWide(desc.Name);
		DxResource->SetName(Conv.c_str());
	}
	*Resource = new GPUResource(DxResource, desc.InitalState, Device);
	(*Resource)->SetDebugName(desc.Name);
	ContainedResources.push_back(*Resource);
	(*Resource)->SetGPUPage(this);
	(*Resource)->Chunk = UsedChunk;
	if (!desc.UseCommittedResource)
	{
		//#dxTODO: might need on pc?
		(*Resource)->SetCurrentAliasState(EPhysicalMemoryState::Active_NoClear);
	}
	if (LogPageAllocations.GetBoolValue())
	{
		//Log::LogMessage("Allocating " + std::to_string(desc.Size / 1e6) + "MB Called '" + desc.Name + "' in Segment " + EGPUMemorysegment::ToString(desc.Segment));
		Log::LogMessage("Page '" + PageDesc.Name + "' Used " + StringUtils::ByteToMB(GetSizeInUse()) + " / " + StringUtils::ByteToMB(PageDesc.Size));
	}
	return EAllocateResult::OK;
}

void GPUMemoryPage::CalculateSpaceNeeded(AllocDesc & desc)
{
	desc.ResourceDesc.Alignment = 0;
	desc.TextureAllocData = Device->GetDevice()->GetResourceAllocationInfo(0, 1, &desc.ResourceDesc);
	desc.ResourceDesc.Alignment = desc.TextureAllocData.Alignment;
}

bool GPUMemoryPage::CheckSpaceForResource(AllocDesc & desc)
{
	if (desc.UseCommittedResource)
	{
		//committed pages are sized as needed by nature
		return false;
	}
	if (desc.PageAllocationType != PageDesc.PageAllocationType)
	{
		return false;
	}
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
		if (FreeChunks[i]->CanFitAllocation(desc, AddressAlignmentForce))
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
	const uint64_t ResourceSize = D3D12Helpers::Align(desc.TextureAllocData.SizeInBytes, AddressAlignmentForce);

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

bool GPUMemoryPage::AllocationChunk::CanFitAllocation(const AllocDesc & desc, uint64 AddressAlignmentForce) const
{
	return size > D3D12Helpers::Align(desc.TextureAllocData.SizeInBytes, AddressAlignmentForce);
}

void GPUMemoryPage::CreateResourceCommitted(AllocationChunk* chunk, AllocDesc & desc, ID3D12Resource** Resource)
{
	D3D12_CLEAR_VALUE* value = nullptr;
	if (desc.ResourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET || desc.ResourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
	{
		value = &desc.ClearValue;
	}
	D3D12_HEAP_FLAGS Flags = desc.HeapFlags;
	if (PageDesc.PageAllocationType == EPageTypes::RTAndDS_Only)
	{
		desc.ResourceDesc.Alignment = 0;	
	}
	ThrowIfFailed(Device->GetDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		Flags,
		&desc.ResourceDesc,
		desc.InitalState,
		value,
		ID_PASS(Resource)));
	AllocationChunk* NewChunk = new AllocationChunk();
	const uint64_t ResourceSize = D3D12Helpers::Align(desc.TextureAllocData.SizeInBytes, AddressAlignmentForce);
	NewChunk->size = ResourceSize;
	AllocatedChunks.push_back(NewChunk);
}

void GPUMemoryPage::CreateResource(AllocationChunk* chunk, AllocDesc & desc, ID3D12Resource** Resource)
{
	D3D12_CLEAR_VALUE* value = nullptr;
	if (desc.ResourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET || desc.ResourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL)
	{
		value = &desc.ClearValue;
	}
#if 1
	//if (PageDesc.PageAllocationType == EPageTypes::BuffersOnly || PageDesc.PageAllocationType == EPageTypes::BufferUploadOnly)
	{
		if (desc.ResourceDesc.Alignment != 0 || desc.ResourceDesc.Alignment != D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT)
		{
			desc.ResourceDesc.Alignment = 0;
		}
	}

	ThrowIfFailed(Device->GetDevice()->CreatePlacedResource(PageHeap, D3D12Helpers::Align(chunk->offset, AddressAlignmentForce), &desc.ResourceDesc, desc.InitalState, value, ID_PASS(Resource)));
#else
	ensure(PageHeap);
	ThrowIfFailed(Device->GetDevice()->CreatePlacedResource(PageHeap, chunk->offset, &desc.ResourceDesc, desc.InitalState, value, ID_PASS(Resource)));
#endif
}

void GPUMemoryPage::InitHeap()
{
	if (PageDesc.UseCommittedResource)
	{
		return;
	}
	if (PageDesc.PageAllocationType == EPageTypes::RTAndDS_Only)
	{
		AddressAlignmentForce = D3D12RHIConfig::RenderTargetMemoryAlignment;
	}
	D3D12_HEAP_DESC desc = {};
	{
		// To avoid wasting memory SizeInBytes should be 
		// multiples of the effective alignment [Microsoft 2018a]
		desc.SizeInBytes = D3D12Helpers::Align(PageDesc.Size, AddressAlignmentForce);
		desc.Alignment = 0;
		if (PageDesc.PageAllocationType == EPageTypes::BufferUploadOnly)
		{
			desc.Properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		}
		else if (PageDesc.PageAllocationType == EPageTypes::ReadBack)
		{
			desc.Properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK);
		}
		else
		{
			desc.Properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		}
		desc.Flags = GetFlagsForType(PageDesc.PageAllocationType);

		ThrowIfFailed(Device->GetDevice()->CreateHeap(&desc, ID_PASS(&PageHeap)));
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
		return D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS | D3D12RHIConfig::IndirectBufferHeapFlag;
	case EPageTypes::TexturesOnly:
		return D3D12_HEAP_FLAG_NONE | D3D12_HEAP_FLAG_DENY_BUFFERS | D3D12_HEAP_FLAG_DENY_RT_DS_TEXTURES;
	case EPageTypes::ReadBack:
		return D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
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
	if (R->Chunk != nullptr)
	{
		VectorUtils::Remove(AllocatedChunks, R->Chunk);
		FreeChunks.push_back(R->Chunk);
	}
	VectorUtils::Remove(ContainedResources, R);
	if (ContainedResources.size() == 0)
	{
		ResetPage();
	}
}

UINT64 GPUMemoryPage::GetSize(bool LocalOnly) const
{
	if (LocalOnly && PageDesc.PageAllocationType == EPageTypes::BufferUploadOnly)
	{
		//not in GPU Local memory
		return 0;
	}
	return PageDesc.Size;
}

UINT64 GPUMemoryPage::GetSizeInUse(bool LocalOnly) const
{
	if (LocalOnly && PageDesc.PageAllocationType == EPageTypes::BufferUploadOnly)
	{
		//not in GPU local memory
		return 0;
	}
	UINT64 Bytes = 0;
	for (int i = 0; i < AllocatedChunks.size(); i++)
	{
		Bytes += AllocatedChunks[i]->size;
	}
	return Bytes;
}

void GPUMemoryPage::LogReport(bool ReportResources)
{
	Log::LogMessage("Page '" + PageDesc.Name + "' Has " + std::to_string(ContainedResources.size()) + " resources using " + StringUtils::ByteToMB(GetSizeInUse()) + " / "
		+ StringUtils::ByteToMB(PageDesc.Size) + " Free Chunks " + std::to_string(FreeChunks.size()));
#if NAME_RHI_PRIMS
	if (ReportResources)
	{
		for (GPUResource* r : ContainedResources)
		{
			Log::LogMessage("	" + StringUtils::ByteToMB(r->Chunk->size) + std::string(" name: '") + std::string(r->GetDebugName()) + "'");
		}
	}
#endif
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



