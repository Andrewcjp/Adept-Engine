#include "HighLevelAccelerationStructure.h"


HighLevelAccelerationStructure::HighLevelAccelerationStructure(DeviceContext* Device, const AccelerationStructureDesc & desc)
{
	Context = Device;
	Desc = desc;
}


HighLevelAccelerationStructure::~HighLevelAccelerationStructure()
{}

void HighLevelAccelerationStructure::AddEntity(LowLevelAccelerationStructure * Struct)
{
	ContainedEntites.push_back(Struct);
}

void HighLevelAccelerationStructure::RemoveEntity(LowLevelAccelerationStructure * Struct)
{
	VectorUtils::Remove(ContainedEntites, Struct);
}

void HighLevelAccelerationStructure::Update(RHICommandList* List)
{

}

void HighLevelAccelerationStructure::Build(RHICommandList * list)
{

}

void HighLevelAccelerationStructure::InitialBuild()
{

}
