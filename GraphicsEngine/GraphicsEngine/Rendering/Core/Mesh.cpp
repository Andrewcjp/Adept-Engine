#include "Mesh.h"
#include "Mesh.h"
#include "../RHI/RHI.h"

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
}


void Mesh::Render(RHICommandList * list)
{
	list->SetVertexBuffer(VertexBuffer);
	list->DrawPrimitive(VertexBuffer->GetVertexCount(), 1, 0, 0);
}

void Mesh::LoadMeshFromFile(std::string filename)
{
	VertexBuffer = RHI::CreateRHIBuffer(RHIBuffer::Vertex);
	VertexBuffer->CreateVertexBufferFromFile(filename);
}
