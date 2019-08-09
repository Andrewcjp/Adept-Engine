#pragma once
struct EGPUMemorysegment
{
	enum Type
	{
		Local,
		CPU_Local,//amd Only
		Host,
		Limit
	};
	static std::string ToString(EGPUMemorysegment::Type T);
};
struct AllocDesc
{
	AllocDesc() {};
	AllocDesc(UINT size, D3D12_RESOURCE_STATES State = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON,const std::string& name = std::string(), EGPUMemorysegment::Type seg = EGPUMemorysegment::Local);
	UINT Size = 0;
	UINT Alignment = 0;
	D3D12_RESOURCE_STATES InitalState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
	EGPUMemorysegment::Type Segment = EGPUMemorysegment::Local;
	std::string Name = std::string();
	
};
namespace EResourceState
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