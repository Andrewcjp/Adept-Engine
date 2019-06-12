#pragma once
#include "Rendering/RayTracing/HighLevelAccelerationStructure.h"
class D3D12HighLevelAccelerationStructure :public HighLevelAccelerationStructure
{
public:
	D3D12HighLevelAccelerationStructure(DeviceContext* Device);
	virtual ~D3D12HighLevelAccelerationStructure();

	virtual void Update(RHICommandList* List) override;
	virtual void Build(RHICommandList* list) override;
	virtual void InitialBuild() override;

	void SetTransfrom(D3D12_RAYTRACING_INSTANCE_DESC & Desc, Transform * T);

	void BuildInstanceBuffer();
	ID3D12Resource* m_topLevelAccelerationStructure = nullptr;
private:
	ID3D12Resource* instanceDescs = nullptr;
	
	ID3D12Resource* scratchSpace = nullptr;
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
};

