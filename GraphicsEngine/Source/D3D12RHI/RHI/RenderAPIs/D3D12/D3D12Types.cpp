#include "D3D12Types.h"

AllocDesc::AllocDesc(UINT64 size, D3D12_RESOURCE_STATES State, D3D12_RESOURCE_FLAGS flags, const std::string& name, EGPUMemorysegment::Type seg)
{
	Size = size;
	InitalState = State;
	Segment = seg;
	Name = name;
	Flags = flags;
}

std::string EGPUMemorysegment::ToString(EGPUMemorysegment::Type T)
{
	switch (T)
	{
		case EGPUMemorysegment::Local:
			return "Local";
		case EGPUMemorysegment::CPU_Local:
			return "CPU_Local";
		case EGPUMemorysegment::Non_Local:
			return "Host";
	}
	return "";
}
