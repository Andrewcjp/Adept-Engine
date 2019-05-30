#pragma once

class Mesh;
class RHICommandList;
class DeviceContext;
class LowLevelAccelerationStructure
{
public:
	RHI_API LowLevelAccelerationStructure(DeviceContext* Device);
	RHI_API virtual ~LowLevelAccelerationStructure();
	RHI_API virtual void CreateFromMesh(Mesh* m);
	RHI_API virtual void Build(RHICommandList* List);
	RHI_API virtual void UpdateTransfrom(Transform* T);
protected:
	DeviceContext* Context = nullptr;
};

