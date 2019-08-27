#include "LowLevelAccelerationStructure.h"


LowLevelAccelerationStructure::LowLevelAccelerationStructure(DeviceContext* Device, const AccelerationStructureDesc & desc)
{
	Context = Device;
	Desc = desc;
}


LowLevelAccelerationStructure::~LowLevelAccelerationStructure()
{
}

void LowLevelAccelerationStructure::CreateFromMesh(Mesh * m)
{
}

void LowLevelAccelerationStructure::Build(RHICommandList * List)
{

}

void LowLevelAccelerationStructure::UpdateTransfrom(Transform * T)
{
	
}

void LowLevelAccelerationStructure::CreateFromEntity(MeshEntity * entity)
{
	
}
