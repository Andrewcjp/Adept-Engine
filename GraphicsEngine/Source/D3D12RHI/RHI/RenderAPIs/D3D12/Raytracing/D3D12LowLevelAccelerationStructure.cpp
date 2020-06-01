#include "D3D12LowLevelAccelerationStructure.h"
#if WIN10_1809
#include "Rendering/Core/Mesh.h"
#include "RHI/RenderAPIs/D3D12/D3D12Buffer.h"
#include "RHI/RenderAPIs/D3D12/D3D12CommandList.h"
#include "RHI/RenderAPIs/D3D12/D3D12DeviceContext.h"
#include "RHI/RenderAPIs/D3D12/D3D12RHI.h"
#include "RHI/RenderAPIs/D3D12/DXMemoryManager.h"
#include "RHI/RenderAPIs/D3D12/GPUResource.h"
#include "RHI/RHICommandList.h"
#include "Rendering/RayTracing/HighLevelAccelerationStructure.h"
#include "D3D12HighLevelAccelerationStructure.h"
#include "Core/Components/Component.h"
#include "Core/GameObject.h"
#include "Core/Components/MeshRendererComponent.h"
#include "RHI/RHIBufferGroup.h"

D3D12LowLevelAccelerationStructure::D3D12LowLevelAccelerationStructure(DeviceContext* Device, const AccelerationStructureDesc & Desc) :LowLevelAccelerationStructure(Device, Desc)
{}


D3D12LowLevelAccelerationStructure::~D3D12LowLevelAccelerationStructure()
{}

void D3D12LowLevelAccelerationStructure::CreateFromEntity(MeshEntity* entity)
{
	AddEntity(entity);
	CreateStructure();
}

void D3D12LowLevelAccelerationStructure::Release()
{
	IRHIResourse::Release();
	SafeRelease(Structure);
	SafeRelease(scratchResource);
}

void D3D12LowLevelAccelerationStructure::CreateFromAABBList(const std::vector<RTAABB>& list)
{
	D3D12_RAYTRACING_AABB* AABBs = new D3D12_RAYTRACING_AABB[list.size()];
	for (int i = 0; i < list.size(); i++)
	{
		AABBs[i].MaxX = list[i].Max.x;
		AABBs[i].MaxY = list[i].Max.y;
		AABBs[i].MaxZ = list[i].Max.z;

		AABBs[i].MinX = list[i].Min.x;
		AABBs[i].MinY = list[i].Min.x;
		AABBs[i].MinZ = list[i].Min.x;
	}
	D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
	geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS;
	geometryDesc.AABBs.AABBCount = list.size();
	AllocDesc D = {};
	D.InitalState = D3D12_RESOURCE_STATE_GENERIC_READ;
	D.ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(D3D12_RAYTRACING_AABB)*list.size());
	D.ResourceDesc.Alignment = D3D12_RAYTRACING_AABB_BYTE_ALIGNMENT;
	D.Name = "AABBs";
	GPUResource* GPUAABBbuffer = nullptr;
	D3D12RHI::DXConv(Context)->GetMemoryManager()->AllocUploadTemporary(D, &GPUAABBbuffer);
	void *pMappedData;
	CD3DX12_RANGE readRange(0, 0);
	GPUAABBbuffer->GetResource()->Map(0, &readRange, &pMappedData);
	memcpy(pMappedData, &AABBs[0], sizeof(D3D12_RAYTRACING_AABB)*list.size());
	GPUAABBbuffer->GetResource()->Unmap(0, nullptr);


	geometryDesc.AABBs.AABBs.StartAddress = GPUAABBbuffer->GetResource()->GetGPUVirtualAddress();
	geometryDesc.AABBs.AABBs.StrideInBytes = sizeof(D3D12_RAYTRACING_AABB);
	geometryDescs.push_back(geometryDesc);
	//geometryDesc.Flags = 
	CreateStructure();
}

void D3D12LowLevelAccelerationStructure::CreateFromMesh(Mesh* m)
{
	//todo: handle Merge sub meshes
	for (int i = 0; i < m->SubMeshes.size(); i++)
	{
		MeshEntity* Entity = m->SubMeshes[i];
		AddEntity(Entity);
	}
	CreateStructure();
}

void D3D12LowLevelAccelerationStructure::CreateStructure()
{
	ensure(geometryDescs.size());
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12HighLevelAccelerationStructure::GetBuildFlags(Desc.BuildFlags);

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &bottomLevelInputs = bottomLevelBuildDesc.Inputs;
	bottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	bottomLevelInputs.Flags = buildFlags;
	bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	bottomLevelInputs.NumDescs = (UINT)geometryDescs.size();
	bottomLevelInputs.pGeometryDescs = &geometryDescs[0];
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
	D3D12RHI::DXConv(Context)->GetDevice5()->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);
	AllocDesc desc = AllocDesc(bottomLevelPrebuildInfo.ScratchDataSizeInBytes, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS, "ScratchResource");
	desc.ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(desc.Size, desc.Flags);
	D3D12RHI::DXConv(Context)->GetMemoryManager()->AllocTemporaryGPU(desc, &scratchResource);

	AllocDesc D = {};
	D.InitalState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
	D.ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	D3D12RHI::DXConv(Context)->GetMemoryManager()->AllocTemporaryGPU(D, &Structure);
	bottomLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->GetResource()->GetGPUVirtualAddress();
	bottomLevelBuildDesc.DestAccelerationStructureData = Structure->GetResource()->GetGPUVirtualAddress();
}

void D3D12LowLevelAccelerationStructure::AddEntity(MeshEntity* Entity)
{
	D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
	geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geometryDesc.Triangles.IndexBuffer = D3D12RHI::DXConv(Entity->IndexBuffers->Get(0))->GetResource()->GetResource()->GetGPUVirtualAddress();
	geometryDesc.Triangles.IndexCount = (UINT)D3D12RHI::DXConv(Entity->IndexBuffers->Get(0))->GetVertexCount();
	geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R16_UINT;
	geometryDesc.Triangles.Transform3x4 = 0;
	geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
	geometryDesc.Triangles.VertexBuffer.StartAddress = D3D12RHI::DXConv(Entity->VertexBuffers->Get(0))->GetResource()->GetResource()->GetGPUVirtualAddress();
	geometryDesc.Triangles.VertexCount = (UINT)D3D12RHI::DXConv(Entity->VertexBuffers->Get(0))->GetVertexCount();
	ensure(geometryDesc.Triangles.VertexCount > 0);
	geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(OGLVertex);
//	geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
	geometryDescs.push_back(geometryDesc);
}

void D3D12LowLevelAccelerationStructure::Build(RHICommandList* List)
{
	D3D12CommandList* DXList = D3D12RHI::DXConv(List);
	DXList->GetCMDList4()->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);

	DXList->GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(Structure->GetResource()));
	LowLevelAccelerationStructure::Build(List);
}

void D3D12LowLevelAccelerationStructure::UpdateTransfrom(Transform* T)
{
	//either rebuild or update.
	Transfrom = T;
}

ID3D12Resource * D3D12LowLevelAccelerationStructure::GetASResource() const
{
	return Structure->GetResource();
}

Transform * D3D12LowLevelAccelerationStructure::GetTransform() const
{
	return Transfrom;
}


#endif