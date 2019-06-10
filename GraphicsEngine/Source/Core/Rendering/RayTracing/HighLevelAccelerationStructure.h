#pragma once

class LowLevelAccelerationStructure;
class RHICommandList;
class HighLevelAccelerationStructure
{
public:
	RHI_API HighLevelAccelerationStructure(DeviceContext* Device);
	RHI_API virtual ~HighLevelAccelerationStructure();
	RHI_API virtual void AddEntity(LowLevelAccelerationStructure* Struct);
	RHI_API virtual void RemoveEntity(LowLevelAccelerationStructure* Struct);

	//Updates this AS depending on mode could rebuild the entire structure.
	RHI_API virtual void Update(RHICommandList* List);
	//Launches the build on the GPU
	RHI_API virtual void Build(RHICommandList* list);
	//called to build this structure the first time needed if using update mode.
	RHI_API virtual void InitialBuild();
protected:
	std::vector<LowLevelAccelerationStructure*> ContainedEntites;
	DeviceContext* Context = nullptr;
};

