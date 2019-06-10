#include "Stdafx.h"
#include "RayTracingCommandList.h"





RayTracingCommandList::RayTracingCommandList(DeviceContext * device, ERayTracingSupportType::Type Mode)
{
	Device = device;
	if (Mode == ERayTracingSupportType::Limit)
	{
		Mode = device->GetCaps().RTSupport;
	}
	CurrentMode = Mode;
	if (UseAPI())
	{
		List = RHI::CreateCommandList(ECommandListType::RayTracing, Device);
	}
	else
	{
		List = RHI::CreateCommandList(ECommandListType::Compute, Device);
	}
}

RayTracingCommandList::~RayTracingCommandList()
{}

void RayTracingCommandList::ResetList()
{
	List->ResetList();
}

void RayTracingCommandList::SetHighLevelAccelerationStructure(HighLevelAccelerationStructure * Struct)
{
	if (UseAPI())
	{
		List->SetHighLevelAccelerationStructure(Struct);
	}
}

void RayTracingCommandList::TraceRays(const RHIRayDispatchDesc & desc)
{
	if (UseAPI())
	{
		List->TraceRays(desc);
	}
}

void RayTracingCommandList::SetStateObject(RHIStateObject * Object)
{
	if (UseAPI())
	{
		List->SetStateObject(Object);
	}
}

void RayTracingCommandList::Execute()
{
	List->Execute();
}

bool RayTracingCommandList::IsFallback() const
{
	return CurrentMode == ERayTracingSupportType::Software;
}

bool RayTracingCommandList::UseAPI() const
{
	return CurrentMode == ERayTracingSupportType::Hardware || CurrentMode == ERayTracingSupportType::DriverBased;
}

RHICommandList * RayTracingCommandList::GetRHIList()
{
	return List;
}
