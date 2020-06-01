#pragma once
#include "Rendering/RayTracing/LowLevelAccelerationStructure.h"
#if WIN10_1809
class GPUResource;
class D3D12LowLevelAccelerationStructure :public LowLevelAccelerationStructure
{
public:
	D3D12LowLevelAccelerationStructure(DeviceContext* Device, const AccelerationStructureDesc & Desc);
	virtual ~D3D12LowLevelAccelerationStructure();
	virtual void CreateFromMesh(Mesh* m) override;

	void CreateStructure();

	void AddEntity(MeshEntity* Entity);

	virtual void Build(RHICommandList* List) override;
	virtual void UpdateTransfrom(Transform* T) override;
	ID3D12Resource* GetASResource() const;
	Transform* GetTransform()const;

	virtual void CreateFromEntity(MeshEntity* entity) override;


	virtual void Release() override;


	void CreateFromAABBList(const std::vector<RTAABB>& list) override;

private:
	
	Transform* Transfrom = nullptr;
	GPUResource* Structure = nullptr;
	//TODO: check if this is need on all 
	GPUResource* scratchResource = nullptr;
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
	std::vector <D3D12_RAYTRACING_GEOMETRY_DESC> geometryDescs;
};
#endif
