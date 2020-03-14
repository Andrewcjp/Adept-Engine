#pragma once
struct EGPUMemorysegment
{
	enum Type
	{
		Local,
		Non_Local,
		Limit
	};
	static std::string ToString(EGPUMemorysegment::Type T);
};
namespace EPageTypes
{
	enum Type
	{
		All,
		RTAndDS_Only,
		BuffersOnly,
		BufferUploadOnly,
		TexturesOnly,
		ReadBack,
		Limit
	};
}
struct AllocDesc
{
	AllocDesc() {};
	AllocDesc(UINT64 size, D3D12_RESOURCE_STATES State = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_FLAGS Flags = D3D12_RESOURCE_FLAG_NONE,
		const std::string& name = std::string(), EGPUMemorysegment::Type seg = EGPUMemorysegment::Local);
	UINT64 Size = 0;
	D3D12_RESOURCE_STATES InitalState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
	EGPUMemorysegment::Type Segment = EGPUMemorysegment::Local;
	std::string Name = std::string();
	D3D12_RESOURCE_FLAGS Flags = D3D12_RESOURCE_FLAG_NONE;
	EPageTypes::Type PageAllocationType = EPageTypes::All;
	D3D12_RESOURCE_ALLOCATION_INFO TextureAllocData;
	D3D12_CLEAR_VALUE ClearValue = {};
	D3D12_RESOURCE_DESC ResourceDesc;
	D3D12_HEAP_FLAGS HeapFlags = D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_NONE;
	bool IsTransient = false;//notify that this resource is used for a short amount of time.
	bool UseCommittedResource = false;
	bool IsReadBack = false;
};
namespace EResourcePageState
{
	enum Type
	{
		Resident,
		Tranfering,
		DestructQueued,
		Evicted
	};
}
namespace EAllocateResult
{
	enum Type
	{
		OK,
		NoSpace,
		WrongSegment,
		APIFailure,
		Limit
	};
}