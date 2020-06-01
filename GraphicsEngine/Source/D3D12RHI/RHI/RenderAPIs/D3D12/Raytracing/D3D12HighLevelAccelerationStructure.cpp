#include "D3D12HighLevelAccelerationStructure.h"
#include "D3D12LowLevelAccelerationStructure.h"
#include "RHI/RenderAPIs/D3D12/D3D12CommandList.h"
#include "RHI/RenderAPIs/D3D12/D3D12DeviceContext.h"
#include "RHI/RenderAPIs/D3D12/D3D12RHI.h"
#include "../DXMemoryManager.h"
#include "../GPUResource.h"
#if WIN10_1809
D3D12HighLevelAccelerationStructure::D3D12HighLevelAccelerationStructure(DeviceContext* Device, const AccelerationStructureDesc & desc) :HighLevelAccelerationStructure(Device, desc)
{}


D3D12HighLevelAccelerationStructure::~D3D12HighLevelAccelerationStructure()
{}

void D3D12HighLevelAccelerationStructure::Update(RHICommandList* List)
{
	ensure(Desc.BuildFlags & AS_BUILD_FLAGS::AllowUpdate);
	BuildInstanceBuffer();

	topLevelBuildDesc.Inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE /*| D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE*/;
	topLevelBuildDesc.Inputs.pGeometryDescs = nullptr;
	topLevelBuildDesc.Inputs.InstanceDescs = instanceDescs->GetResource()->GetGPUVirtualAddress();
	topLevelBuildDesc.Inputs.NumDescs = GetValidEntites();
	topLevelBuildDesc.SourceAccelerationStructureData = m_topLevelAccelerationStructure->GetResource()->GetGPUVirtualAddress();
	AllocateSpace(topLevelBuildDesc.Inputs.Flags, topLevelBuildDesc.Inputs.NumDescs);
	D3D12CommandList* DXList = D3D12RHI::DXConv(List);
	DXList->GetCMDList4()->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);

	DXList->GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_topLevelAccelerationStructure->GetResource()));
}

void D3D12HighLevelAccelerationStructure::Build(RHICommandList* list)
{
	//InitialBuild();
	BuildInstanceBuffer();
	//ensure(instanceDescs);
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = GetBuildFlags(Desc.BuildFlags);

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &topLevelInputs = topLevelBuildDesc.Inputs;
	topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	topLevelInputs.Flags = buildFlags;
	topLevelInputs.pGeometryDescs = nullptr;
	topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	topLevelBuildDesc.Inputs.NumDescs = GetValidEntites();
	AllocateSpace(buildFlags, GetValidEntites());
	topLevelBuildDesc.Inputs.InstanceDescs = instanceDescs->GetResource()->GetGPUVirtualAddress();
	D3D12CommandList* DXList = D3D12RHI::DXConv(list);
	DXList->GetCMDList4()->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);

	DXList->GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(m_topLevelAccelerationStructure->GetResource()));
}

void D3D12HighLevelAccelerationStructure::AllocateSpace(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS BuildFlags, uint DescCount)
{
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = BuildFlags;

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &topLevelInputs = topLevelBuildDesc.Inputs;
	topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	topLevelInputs.Flags = buildFlags;
	topLevelInputs.NumDescs = DescCount;
	topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {};

	D3D12RHI::DXConv(Context)->GetDevice5()->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);

	if (scratchSpace == nullptr || scratchSpace->GetResource()->GetDesc().Width < topLevelPrebuildInfo.ScratchDataSizeInBytes)
	{
		EnqueueSafeRHIRelease(scratchSpace);
		AllocDesc desc = AllocDesc(topLevelPrebuildInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, "ScratchResource");
		desc.ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(desc.Size, desc.Flags);
		D3D12RHI::DXConv(Context)->GetMemoryManager()->AllocTemporaryGPU(desc, &scratchSpace);
	}

	if (m_topLevelAccelerationStructure == nullptr || m_topLevelAccelerationStructure->GetResource()->GetDesc().Width < topLevelPrebuildInfo.ResultDataMaxSizeInBytes)
	{
		EnqueueSafeRHIRelease(m_topLevelAccelerationStructure);
		AllocDesc desc = AllocDesc(topLevelPrebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, "TopLevelAccelerationStructure");
		desc.ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(desc.Size, desc.Flags);
		D3D12RHI::DXConv(Context)->GetMemoryManager()->AllocResource(desc, &m_topLevelAccelerationStructure);
	}

	topLevelBuildDesc.ScratchAccelerationStructureData = scratchSpace->GetResource()->GetGPUVirtualAddress();
	topLevelBuildDesc.DestAccelerationStructureData = m_topLevelAccelerationStructure->GetResource()->GetGPUVirtualAddress();
}

void D3D12HighLevelAccelerationStructure::InitialBuild()
{
	AllocateSpace(GetBuildFlags(Desc.BuildFlags), 1500);
	BuildInstanceBuffer();
}

void D3D12HighLevelAccelerationStructure::SetTransfrom(D3D12_RAYTRACING_INSTANCE_DESC& iDesc, Transform* T)
{
	glm::mat4 Model = transpose(T->GetModel());
	for (int x = 0; x < 3; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			iDesc.Transform[x][y] = Model[x][y];
		}
	}
}

void D3D12HighLevelAccelerationStructure::BuildInstanceBuffer()
{
	std::vector<D3D12_RAYTRACING_INSTANCE_DESC> Descs;
#if 0
	for (int i = 0; i < ContainedEntites.size(); i++)
	{
		if (!ContainedEntites[i]->IsValid())
		{
			continue;
		}
		/*if (i >= topLevelBuildDesc.Inputs.NumDescs)
		{
			break;
		}*/
		D3D12LowLevelAccelerationStructure* E = D3D12RHI::DXConv(ContainedEntites[i]);
		D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
		SetTransfrom(instanceDesc, E->GetTransform());
		instanceDesc.InstanceMask = E->LayerMask;
		//instanceDesc.InstanceID = i;
		instanceDesc.InstanceContributionToHitGroupIndex = i;
		instanceDesc.AccelerationStructure = E->GetASResource()->GetGPUVirtualAddress();
		Descs.push_back(instanceDesc);
	}
#else
	for (int i = 0; i < InstanceDescritors.size(); i++)
	{
		D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
		glm::mat4 Model = transpose(InstanceDescritors[i].Transform);
		for (int x = 0; x < 3; x++)
		{
			for (int y = 0; y < 4; y++)
			{
				instanceDesc.Transform[x][y] = Model[x][y];
			}
		}
		instanceDesc.InstanceContributionToHitGroupIndex = InstanceDescritors[i].HitGroupOffset;
		instanceDesc.Flags = InstanceDescritors[i].Flags;
		instanceDesc.InstanceID = InstanceDescritors[i].InstanceID;
		instanceDesc.InstanceMask = InstanceDescritors[i].Mask;
		D3D12LowLevelAccelerationStructure* E = D3D12RHI::DXConv(InstanceDescritors[i].Structure);
		instanceDesc.AccelerationStructure = E->GetASResource()->GetGPUVirtualAddress();
		Descs.push_back(instanceDesc);
	}
#endif
	if (Descs.size() == 0)
	{
		return;
	}
	if (LAstCount != Descs.size())
	{
		AllocDesc D = {};
		D.InitalState = D3D12_RESOURCE_STATE_GENERIC_READ;
		D.ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(D3D12_RAYTRACING_INSTANCE_DESC)*Descs.size());
		D.Name = "InstanceDescs";
		D3D12RHI::DXConv(Context)->GetMemoryManager()->AllocUploadTemporary(D, &instanceDescs);
		LAstCount = Descs.size();
	}

	void *pMappedData;
	CD3DX12_RANGE readRange(0, 0);
	instanceDescs->GetResource()->Map(0, &readRange, &pMappedData);
	memcpy(pMappedData, &Descs[0], sizeof(D3D12_RAYTRACING_INSTANCE_DESC)*Descs.size());
	instanceDescs->GetResource()->Unmap(0, nullptr);	
}

void D3D12HighLevelAccelerationStructure::Release()
{
	SafeRelease(instanceDescs);
	SafeRelease(scratchSpace);
}

D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS D3D12HighLevelAccelerationStructure::GetBuildFlags(int BuildFlags)
{
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS Flags;
	ensureMsgf(BuildFlags & AS_BUILD_FLAGS::Fast_Build || BuildFlags & AS_BUILD_FLAGS::Fast_Trace, "Both Fast trace and fast build flags set, this is not allowed");
	if (BuildFlags & AS_BUILD_FLAGS::Fast_Build)
	{
		Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
	}
	if (BuildFlags & AS_BUILD_FLAGS::Fast_Trace)
	{
		Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	}
	if (BuildFlags & AS_BUILD_FLAGS::AllowUpdate)
	{
		//xs complie issues?
		//Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
	}
	return Flags;
}
#endif