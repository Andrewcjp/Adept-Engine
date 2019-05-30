#pragma once
#include "Rendering/RayTracing/LowLevelAccelerationStructure.h"
class D3D12LowLevelAccelerationStructure :public LowLevelAccelerationStructure
{
public:
	D3D12LowLevelAccelerationStructure(DeviceContext* Device);
	virtual ~D3D12LowLevelAccelerationStructure();
	virtual void CreateFromMesh(Mesh* m) override;
	virtual void Build(RHICommandList* List) override;
	virtual void UpdateTransfrom(Transform* T) override;
	ID3D12Resource* GetASResource() const;
	Transform* GetTransform()const;
private:
	Transform* Transfrom;
	ID3D12Resource* Structure = nullptr;
	//TODO: check if this is need on all 
	ID3D12Resource* scratchResource = nullptr;
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
	std::vector <D3D12_RAYTRACING_GEOMETRY_DESC> geometryDescs;
};

