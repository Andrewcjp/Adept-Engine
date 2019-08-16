#include "D3D12RHIPCH.h"
#include "GPUMemoryPage.h"
#include "Core/Utils/StringUtil.h"
#include "D3D12Helpers.h"
#include "GPUResource.h"
#include "D3D12DeviceContext.h"
static ConsoleVariable LogPageAllocations("VMEM.LogAlloc", 1, ECVarType::ConsoleAndLaunch);
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
	CreateResource(desc, &DxResource);
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
		Log::LogMessage("Page '" + PageDesc.Name + "' Used " + StringUtils::ByteToMB(OffsetInPlacedHeap) + "/" + StringUtils::ByteToMB(PageDesc.Size) + " MB");
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
	if (OffsetInPlacedHeap + desc.TextureAllocData.SizeInBytes >= PageDesc.Size)
	{
		return false;
	}

	return true;
}

void GPUMemoryPage::CreateResource(AllocDesc & desc, ID3D12Resource** Resource)
{
	D3D12_CLEAR_VALUE* value = nullptr;
	if (desc.ResourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER)
	{
		value = &desc.ClearValue;
	}
	ThrowIfFailed(Device->GetDevice()->CreatePlacedResource(PageHeap, OffsetInPlacedHeap, &desc.ResourceDesc, desc.InitalState, value, IID_PPV_ARGS(Resource)));
	OffsetInPlacedHeap += D3D12Helpers::Align(desc.TextureAllocData.SizeInBytes);
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
}

UINT GPUMemoryPage::GetSize() const
{
	return PageDesc.Size;
}

void GPUMemoryPage::LogReport()
{
	Log::LogMessage("Page '" + PageDesc.Name + "' Has " + std::to_string(ContainedResources.size()) + " resources using " + StringUtils::ByteToMB(OffsetInPlacedHeap) + "/ " + StringUtils::ByteToMB(PageDesc.Size));
}
