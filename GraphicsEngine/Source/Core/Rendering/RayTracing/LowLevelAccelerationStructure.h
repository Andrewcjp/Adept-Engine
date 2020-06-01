#pragma once
#include "RHI/RHITypes.h"
struct RTAABB
{
	RTAABB(glm::vec3 min, glm::vec3 max)
	{
		Min = min;
		Max = max;
	}
	glm::vec3 Min;
	glm::vec3 Max;
};
class Mesh;
class RHICommandList;
class DeviceContext;
class LowLevelAccelerationStructure :public IRHIResourse
{
public:
	RHI_API LowLevelAccelerationStructure(DeviceContext* Device, const AccelerationStructureDesc & Desc);
	RHI_API virtual ~LowLevelAccelerationStructure();
	RHI_API virtual void CreateFromMesh(Mesh* m);
	RHI_API virtual void Build(RHICommandList* List);
	RHI_API virtual void UpdateTransfrom(Transform* T);
	RHI_API virtual void CreateFromEntity(MeshEntity* entity);
	RHI_API virtual void CreateFromAABBList(const std::vector<RTAABB>& list);
	uint LayerMask = 0xFF;
	RHI_API bool IsDirty()const ;
	RHI_API void MarkDirty();
	RHI_API bool IsValid()const;
	bool HasEverBuilt = false;
protected:
	bool NeedsBuild = false;
	DeviceContext* Context = nullptr;
	AccelerationStructureDesc Desc;
};

