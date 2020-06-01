#include "LowLevelAccelerationStructure.h"


LowLevelAccelerationStructure::LowLevelAccelerationStructure(DeviceContext* Device, const AccelerationStructureDesc & desc)
{
	Context = Device;
	Desc = desc;
	MarkDirty();
}


LowLevelAccelerationStructure::~LowLevelAccelerationStructure()
{
}

void LowLevelAccelerationStructure::CreateFromMesh(Mesh * m)
{
}

void LowLevelAccelerationStructure::Build(RHICommandList * List)
{
	HasEverBuilt = true;
	NeedsBuild = false;
}

void LowLevelAccelerationStructure::UpdateTransfrom(Transform * T)
{
	
}

void LowLevelAccelerationStructure::CreateFromEntity(MeshEntity * entity)
{
	
}

void LowLevelAccelerationStructure::CreateFromAABBList(const std::vector<RTAABB>& list)
{
	
}

bool LowLevelAccelerationStructure::IsDirty() const
{
	return NeedsBuild;
}

void LowLevelAccelerationStructure::MarkDirty()
{
	NeedsBuild = true;
}

bool LowLevelAccelerationStructure::IsValid() const
{
	return HasEverBuilt;
}
