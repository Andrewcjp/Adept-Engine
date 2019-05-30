#include "Stdafx.h"
#include "HighLevelAccelerationStructure.h"


HighLevelAccelerationStructure::HighLevelAccelerationStructure(DeviceContext* Device)
{
	Context = Device;
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

void HighLevelAccelerationStructure::Update()
{}

void HighLevelAccelerationStructure::Build(RHICommandList * list)
{

}

void HighLevelAccelerationStructure::InitialBuild()
{

}
