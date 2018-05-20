#include "Mesh.h"
#include "Mesh.h"
#include "../RHI/RHI.h"
#include "../Core/Assets/MeshLoader.h"
Mesh::Mesh()
{

}

Mesh::Mesh(std::string filename, MeshLoader::FMeshLoadingSettings& Settings)
{
	LoadMeshFromFile(filename, Settings);
}

Mesh::~Mesh()
{
	delete VertexBuffer;
	delete IndexBuffer;
}

void Mesh::Render(RHICommandList * list)
{
	list->SetVertexBuffer(VertexBuffer);
	list->SetIndexBuffer(IndexBuffer);
	list->DrawPrimitive(VertexBuffer->GetVertexCount(), 1, 0, 0);
}

void Mesh::LoadMeshFromFile(std::string filename, MeshLoader::FMeshLoadingSettings& Settings)
{
	VertexBuffer = RHI::CreateRHIBuffer(RHIBuffer::Vertex);
	IndexBuffer = RHI::CreateRHIBuffer(RHIBuffer::Index);
	std::vector<OGLVertex> vertices;
	std::vector<int> indices;
	MeshLoader::LoadMeshFromFile(filename,Settings, vertices, indices);
	VertexBuffer->CreateVertexBuffer(sizeof(OGLVertex), sizeof(OGLVertex)* vertices.size(),RHIBuffer::BufferAccessType::Static);
	VertexBuffer->UpdateVertexBuffer(vertices.data(), vertices.size());
	IndexBuffer->CreateIndexBuffer(sizeof(int), sizeof(int)* indices.size());
	IndexBuffer->UpdateIndexBuffer(indices.data(), indices.size());
	
}
