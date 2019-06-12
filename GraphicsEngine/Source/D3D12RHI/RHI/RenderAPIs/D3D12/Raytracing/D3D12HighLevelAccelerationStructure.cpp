#include "D3D12HighLevelAccelerationStructure.h"
#include "D3D12LowLevelAccelerationStructure.h"
#include "RHI/RenderAPIs/D3D12/D3D12CommandList.h"
#include "RHI/RenderAPIs/D3D12/D3D12DeviceContext.h"
#include "RHI/RenderAPIs/D3D12/D3D12RHI.h"

D3D12HighLevelAccelerationStructure::D3D12HighLevelAccelerationStructure(DeviceContext* Device) :HighLevelAccelerationStructure(Device)
{}


D3D12HighLevelAccelerationStructure::~D3D12HighLevelAccelerationStructure()
{}

void D3D12HighLevelAccelerationStructure::Update(RHICommandList* List)
{
	return;
	BuildInstanceBuffer();

	topLevelBuildDesc.Inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
	topLevelBuildDesc.Inputs.pGeometryDescs = nullptr;
	topLevelBuildDesc.Inputs.InstanceDescs = instanceDescs->GetGPUVirtualAddress();
	topLevelBuildDesc.Inputs.NumDescs = ContainedEntites.size();
	topLevelBuildDesc.SourceAccelerationStructureData = m_topLevelAccelerationStructure->GetGPUVirtualAddress();
	D3D12CommandList* DXList = D3D12RHI::DXConv(List);
	DXList->GetCMDList4()->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);
}

void D3D12HighLevelAccelerationStructure::Build(RHICommandList* list)
{
	ensure(instanceDescs);
	topLevelBuildDesc.DestAccelerationStructureData = m_topLevelAccelerationStructure->GetGPUVirtualAddress();
	topLevelBuildDesc.ScratchAccelerationStructureData = scratchSpace->GetGPUVirtualAddress();
	topLevelBuildDesc.Inputs.InstanceDescs = instanceDescs->GetGPUVirtualAddress();
	topLevelBuildDesc.Inputs.NumDescs = ContainedEntites.size();
	D3D12CommandList* DXList = D3D12RHI::DXConv(list);
	DXList->GetCMDList4()->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);
}

void D3D12HighLevelAccelerationStructure::InitialBuild()
{
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &topLevelInputs = topLevelBuildDesc.Inputs;
	topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	topLevelInputs.Flags = buildFlags;
	topLevelInputs.NumDescs = 1;
	topLevelInputs.pGeometryDescs = nullptr;
	topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};

	D3D12RHI::DXConv(Context)->GetDevice5()->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);

	D3D12Helpers::AllocateUAVBuffer(D3D12RHI::DXConv(Context)->GetDevice(),
		topLevelPrebuildInfo.ScratchDataSizeInBytes, &scratchSpace, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"ScratchResource");

	D3D12Helpers::AllocateUAVBuffer(D3D12RHI::DXConv(Context)->GetDevice(), topLevelPrebuildInfo.ResultDataMaxSizeInBytes, &m_topLevelAccelerationStructure,
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, L"TopLevelAccelerationStructure");

	topLevelBuildDesc.DestAccelerationStructureData = m_topLevelAccelerationStructure->GetGPUVirtualAddress();
	topLevelBuildDesc.ScratchAccelerationStructureData = scratchSpace->GetGPUVirtualAddress();
	BuildInstanceBuffer();
}
void D3D12HighLevelAccelerationStructure::SetTransfrom(D3D12_RAYTRACING_INSTANCE_DESC& Desc, Transform* T)
{
	glm::mat4 Model = transpose(T->GetModel());
	for (int x = 0; x < 3; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			Desc.Transform[x][y] = Model[x][y];
		}
	}
}

void D3D12HighLevelAccelerationStructure::BuildInstanceBuffer()
{
	std::vector<D3D12_RAYTRACING_INSTANCE_DESC> Descs;
	for (int i = 0; i < ContainedEntites.size(); i++)
	{
		D3D12LowLevelAccelerationStructure* E = D3D12RHI::DXConv(ContainedEntites[i]);
		D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
		SetTransfrom(instanceDesc, E->GetTransform());
		instanceDesc.InstanceMask = 1;
		instanceDesc.InstanceID = i;
		instanceDesc.InstanceContributionToHitGroupIndex = i;
		instanceDesc.AccelerationStructure = E->GetASResource()->GetGPUVirtualAddress();
		Descs.push_back(instanceDesc);
	}
	if (instanceDescs == nullptr)
	{
		D3D12Helpers::AllocateUploadBuffer(D3D12RHI::DXConv(Context)->GetDevice(), &Descs[0], sizeof(D3D12_RAYTRACING_INSTANCE_DESC)*Descs.size(), &instanceDescs, L"InstanceDescs");
	}
	else
	{
		/*void *pMappedData;
		instanceDescs->Map(0, nullptr, &pMappedData);
		memcpy(pMappedData, &Descs[0], sizeof(D3D12_RAYTRACING_INSTANCE_DESC)*Descs.size());
		instanceDescs->Unmap(0, nullptr);*/
	}
}