#pragma once
#include "RHI/RHITypes.h"

class LowLevelAccelerationStructure;
class RHICommandList;
namespace RAYTRACING_INSTANCE_FLAGS
{
	enum Type
	{
		RAYTRACING_INSTANCE_FLAG_NONE = 0,
		RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE = 0x1,
		RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE = 0x2,
		RAYTRACING_INSTANCE_FLAG_FORCE_OPAQUE = 0x4,
		RAYTRACING_INSTANCE_FLAG_FORCE_NON_OPAQUE = 0x8
	};
}
struct AccelerationStructureInstanceDesc
{
	glm::mat4 Transform;
	uint HitGroupOffset = 0;
	uint Mask = 0xFF;
	uint InstanceID = 0;
	RAYTRACING_INSTANCE_FLAGS::Type Flags = RAYTRACING_INSTANCE_FLAGS::RAYTRACING_INSTANCE_FLAG_NONE;
	LowLevelAccelerationStructure* Structure = nullptr;
};
class HighLevelAccelerationStructure :public IRHIResourse
{
public:
	RHI_API HighLevelAccelerationStructure(DeviceContext* Device, const AccelerationStructureDesc & desc);
	RHI_API virtual ~HighLevelAccelerationStructure();
	RHI_API virtual void AddEntity(LowLevelAccelerationStructure* Struct);
	RHI_API virtual void RemoveEntity(LowLevelAccelerationStructure* Struct);
	RHI_API virtual void ClearEntitys();
	RHI_API void AddInstance(const AccelerationStructureInstanceDesc& desc);
	RHI_API void RemoveInstance(const AccelerationStructureInstanceDesc& desc);
	RHI_API void ClearInstances();
	//Updates this AS depending on mode could rebuild the entire structure.
	RHI_API virtual void Update(RHICommandList* List);
	//Launches the build on the GPU
	RHI_API virtual void Build(RHICommandList* list);
	//called to build this structure the first time needed if using update mode.
	RHI_API virtual void InitialBuild();
	RHI_API int GetValidEntites()const;
protected:
	std::vector<LowLevelAccelerationStructure*> ContainedEntites;
	std::vector<AccelerationStructureInstanceDesc> InstanceDescritors;
	DeviceContext* Context = nullptr;
	AccelerationStructureDesc Desc;
};

