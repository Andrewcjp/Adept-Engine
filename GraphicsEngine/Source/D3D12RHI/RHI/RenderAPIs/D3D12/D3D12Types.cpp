#include "D3D12Types.h"

AllocDesc::AllocDesc(UINT size, D3D12_RESOURCE_STATES State, const std::string& name, EGPUMemorysegment::Type seg)
{
	Size = size;
	InitalState = State;
	Segment = seg;
	Name = name;
}

std::string EGPUMemorysegment::ToString(EGPUMemorysegment::Type T)
{
	switch (T)
	{
		case EGPUMemorysegment::Local:
			return "Local";
		case EGPUMemorysegment::CPU_Local:
			return "CPU_Local";
		case EGPUMemorysegment::Host:
			return "Host";
	}
	return "";
}
