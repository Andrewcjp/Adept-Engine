#include "D3D12LowLevelAccelerationStructure.h"
#include "RHI/RenderAPIs/D3D12/D3D12DeviceContext.h"
#include "RHI/RenderAPIs/D3D12/D3D12RHI.h"
#include "../D3D12CommandList.h"
#include "../GPUResource.h"

D3D12LowLevelAccelerationStructure::D3D12LowLevelAccelerationStructure(DeviceContext* Device) :LowLevelAccelerationStructure(Device)
{}


D3D12LowLevelAccelerationStructure::~D3D12LowLevelAccelerationStructure()
{}

void D3D12LowLevelAccelerationStructure::CreateFromMesh(Mesh* m)
{
	//todo: handle Merge sub meshes


	for (int i = 0; i < m->SubMeshes.size(); i++)
	{
		MeshEntity* Entity = m->SubMeshes[i];
		D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
		geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		geometryDesc.Triangles.IndexBuffer = D3D12RHI::DXConv(Entity->IndexBuffers[0].Get())->GetResource()->GetResource()->GetGPUVirtualAddress();
		geometryDesc.Triangles.IndexCount = D3D12RHI::DXConv(Entity->IndexBuffers[0].Get())->GetVertexCount();
		geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R16_UINT;
		geometryDesc.Triangles.Transform3x4 = 0;
		geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
		geometryDesc.Triangles.VertexBuffer.StartAddress = D3D12RHI::DXConv(Entity->VertexBuffers[0].Get())->GetResource()->GetResource()->GetGPUVirtualAddress();
		geometryDesc.Triangles.VertexCount = D3D12RHI::DXConv(Entity->VertexBuffers[0].Get())->GetVertexCount();
		geometryDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(OGLVertex);
		geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
		geometryDescs.push_back(geometryDesc);
	}

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;


	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS &bottomLevelInputs = bottomLevelBuildDesc.Inputs;
	bottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	bottomLevelInputs.Flags = buildFlags;

	bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	bottomLevelInputs.NumDescs = geometryDescs.size();
	bottomLevelInputs.pGeometryDescs = &geometryDescs[0];
	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {};
	D3D12RHI::DXConv(Context)->GetDevice5()->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);

	D3D12Helpers::AllocateUAVBuffer(D3D12RHI::DXConv(Context)->GetDevice(),
		bottomLevelPrebuildInfo.ScratchDataSizeInBytes, &scratchResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, L"ScratchResource");

	D3D12Helpers::AllocateUAVBuffer(D3D12RHI::DXConv(Context)->GetDevice(), bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes, &Structure, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, L"BottomLevelAccelerationStructure");

	bottomLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();
	bottomLevelBuildDesc.DestAccelerationStructureData = Structure->GetGPUVirtualAddress();

}

void D3D12LowLevelAccelerationStructure::Build(RHICommandList* List)
{
	D3D12CommandList* DXList = D3D12RHI::DXConv(List);
	DXList->GetCMDList4()->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);
	//#DXR: DXList->UAVBarrier()?
}

void D3D12LowLevelAccelerationStructure::UpdateTransfrom(Transform* T)
{
	//either rebuild or update.
	Transfrom = T;
}

ID3D12Resource * D3D12LowLevelAccelerationStructure::GetASResource() const
{
	return Structure;
}

Transform * D3D12LowLevelAccelerationStructure::GetTransform() const
{
	return Transfrom;
}
