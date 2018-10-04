#include "Mesh.h"
#include "Mesh.h"
#include "RHI/RHI.h"
#include "Core/Assets/MeshLoader.h"
#include "RHI/DeviceContext.h"
#include "Core/Utils/MemoryUtils.h"
Mesh::Mesh()
{}

Mesh::Mesh(std::string filename, MeshLoader::FMeshLoadingSettings& Settings)
{
	LoadMeshFromFile(filename, Settings);
	FrameCreated = RHI::GetFrameCount();
	if (FrameCreated == 0)
	{
		FrameCreated = -10;
	}
}

void Mesh::Release()
{
	IRHIResourse::Release();
	MemoryUtils::DeleteReleaseableCArray(VertexBuffers, MAX_GPU_DEVICE_COUNT);
	MemoryUtils::DeleteReleaseableCArray(IndexBuffers, MAX_GPU_DEVICE_COUNT);
}

Mesh::~Mesh()
{}

void Mesh::Render(RHICommandList * list)
{
	if (RHI::GetFrameCount() > FrameCreated + 1)
	{
		list->SetVertexBuffer(VertexBuffers[list->GetDeviceIndex()]);
		list->SetIndexBuffer(IndexBuffers[list->GetDeviceIndex()]);
		list->DrawPrimitive((int)VertexBuffers[list->GetDeviceIndex()]->GetVertexCount(), 1, 0, 0);
	}
}

void Mesh::LoadMeshFromFile(std::string filename, MeshLoader::FMeshLoadingSettings& Settings)
{

	std::vector<OGLVertex> vertices;
	std::vector<int> indices;
	MeshLoader::LoadMeshFromFile(filename, Settings, vertices, indices);

	const int count = Settings.InitOnAllDevices ? RHI::GetDeviceCount() : 1;
	for (int i = 0; i < count; i++)
	{
		VertexBuffers[i] = RHI::CreateRHIBuffer(ERHIBufferType::Vertex, RHI::GetDeviceContext(i));
		IndexBuffers[i] = RHI::CreateRHIBuffer(ERHIBufferType::Index, RHI::GetDeviceContext(i));
		VertexBuffers[i]->CreateVertexBuffer(sizeof(OGLVertex), sizeof(OGLVertex)* (int)vertices.size(), EBufferAccessType::Static);
		VertexBuffers[i]->UpdateVertexBuffer(vertices.data(), vertices.size());
		IndexBuffers[i]->CreateIndexBuffer(sizeof(int), sizeof(int)* (int)indices.size());
		IndexBuffers[i]->UpdateIndexBuffer(indices.data(), indices.size());
	}
	indices.clear();
	vertices.clear();

}
