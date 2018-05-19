#include "Mesh.h"
#include "Mesh.h"
#include "../RHI/RHI.h"
#include "../Core/Assets/MeshLoader.h"
Mesh::Mesh()
{

}

Mesh::Mesh(std::string filename)
{
	LoadMeshFromFile(filename);
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

void Mesh::LoadMeshFromFile(std::string filename)
{
	VertexBuffer = RHI::CreateRHIBuffer(RHIBuffer::Vertex);
	IndexBuffer = RHI::CreateRHIBuffer(RHIBuffer::Index);
	std::vector<OGLVertex> vertices;
	std::vector<int> indices;
	MeshLoader::LoadMeshFromFile(filename, vertices, indices);
	VertexBuffer->CreateVertexBuffer(sizeof(OGLVertex), sizeof(OGLVertex)* vertices.size(),RHIBuffer::BufferAccessType::Static);
	VertexBuffer->UpdateVertexBuffer(vertices.data(), vertices.size());
	IndexBuffer->CreateIndexBuffer(sizeof(int), sizeof(int)* indices.size());
	IndexBuffer->UpdateIndexBuffer(indices.data(), indices.size());
	
}
