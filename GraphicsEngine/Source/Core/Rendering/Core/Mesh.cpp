#include "Mesh.h"
#include "Mesh.h"
#include "RHI/RHI.h"
#include "Core/Assets/MeshLoader.h"
#include "RHI/DeviceContext.h"
#include "Core/Utils/MemoryUtils.h"
Mesh::Mesh()
{

}

Mesh::Mesh(std::string filename, MeshLoader::FMeshLoadingSettings& Settings)
{
	LoadMeshFromFile(filename, Settings);
}

Mesh::~Mesh()
{
	MemoryUtils::DeleteCArray(VertexBuffers, MAX_DEVICE_COUNT);
	MemoryUtils::DeleteCArray(IndexBuffers, MAX_DEVICE_COUNT);
}

void Mesh::Render(RHICommandList * list)
{
	list->SetVertexBuffer(VertexBuffers[list->GetDeviceIndex()]);
	list->SetIndexBuffer(IndexBuffers[list->GetDeviceIndex()]);
	list->DrawPrimitive(VertexBuffers[list->GetDeviceIndex()]->GetVertexCount(), 1, 0, 0);
}

void Mesh::LoadMeshFromFile(std::string filename, MeshLoader::FMeshLoadingSettings& Settings)
{

	std::vector<OGLVertex> vertices;
	std::vector<int> indices;
	MeshLoader::LoadMeshFromFile(filename, Settings, vertices, indices);

	const int count = Settings.InitOnAllDevices ? RHI::GetDeviceCount() : 1;
	for (int i = 0; i < count; i++)
	{
		VertexBuffers[i] = RHI::CreateRHIBuffer(RHIBuffer::Vertex, RHI::GetDeviceContext(i));
		IndexBuffers[i] = RHI::CreateRHIBuffer(RHIBuffer::Index, RHI::GetDeviceContext(i));
		VertexBuffers[i]->CreateVertexBuffer(sizeof(OGLVertex), sizeof(OGLVertex)* vertices.size(), RHIBuffer::BufferAccessType::Static);
		VertexBuffers[i]->UpdateVertexBuffer(vertices.data(), vertices.size());
		IndexBuffers[i]->CreateIndexBuffer(sizeof(int), sizeof(int)* indices.size());
		IndexBuffers[i]->UpdateIndexBuffer(indices.data(), indices.size());
	}
}
