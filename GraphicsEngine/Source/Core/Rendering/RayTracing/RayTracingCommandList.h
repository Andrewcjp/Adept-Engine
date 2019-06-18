#pragma once
#include "RHI/DeviceContext.h"

class RHICommandList;
//This abstracts DXR and the Software fall back layer
class RayTracingCommandList
{
public:
	RayTracingCommandList(DeviceContext* Device, ERayTracingSupportType::Type Mode = ERayTracingSupportType::Limit);
	~RayTracingCommandList();
	void ResetList();
	void SetHighLevelAccelerationStructure(HighLevelAccelerationStructure* Struct);
	void TraceRays(const RHIRayDispatchDesc& desc);
	void SetStateObject(RHIStateObject* Object);
	void Execute();
	bool IsFallback() const;
	bool UseAPI()const;
	RHICommandList* GetRHIList();
	void Release();
private:
	RHICommandList* List = nullptr;
	ERayTracingSupportType::Type CurrentMode = ERayTracingSupportType::Limit;
	DeviceContext* Device;
};

