#pragma once
#include "Rendering/RayTracing/HighLevelAccelerationStructure.h"
class D3D12HighLevelAccelerationStructure :public HighLevelAccelerationStructure
{
public:
	D3D12HighLevelAccelerationStructure(DeviceContext* Device, const AccelerationStructureDesc & desc);
	virtual ~D3D12HighLevelAccelerationStructure();

	virtual void Update(RHICommandList* List) override;
	virtual void Build(RHICommandList* list) override;
	void AllocateSpace(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS BuildFlags, uint DescCount);
	virtual void InitialBuild() override;

	void SetTransfrom(D3D12_RAYTRACING_INSTANCE_DESC & Desc, Transform * T);

	void BuildInstanceBuffer();
	GPUResource* m_topLevelAccelerationStructure = nullptr;

	virtual void Release() override;
	static D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS GetBuildFlags(int BuildFlags);
private:
	ID3D12Resource* instanceDescs = nullptr;
	
	GPUResource* scratchSpace = nullptr;
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
};

