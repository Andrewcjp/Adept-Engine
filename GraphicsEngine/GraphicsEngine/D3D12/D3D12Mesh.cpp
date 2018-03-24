#include "stdafx.h"
#include "D3D12Mesh.h"
#include "D3D12RHI.h"
#include "Rendering/Core/Triangle.h"
#include "../Core/Assets/OBJFileReader.h"
#include "../RHI/RHI.h"
#include "../Core/Utils/StringUtil.h"


D3D12Mesh::D3D12Mesh(const char * file)
{
	CreateVertexBuffer(StringUtils::ConvertStringToWide(file).c_str());
}


D3D12Mesh::~D3D12Mesh()
{
}


void D3D12Mesh::Render(RHICommandList * list)
{
	if (list == nullptr && RHI::GetType() == RenderSystemD3D12)
	{
		printf("Error Null List\n");
		return;
	}
	/*list->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	
	list->DrawInstanced(m_numtriangles*3, 1, 0, 0);*/
}
void D3D12Mesh::CreateVertexBuffer(LPCWSTR name)
{

	// Create the vertex buffer.
	{
		
		Triangle* mesh;
		m_numtriangles = importOBJMesh(name, &mesh);
		const UINT vertexBufferSize = sizeof(OGLVertex)*m_numtriangles * 3;

		// Note: using upload heaps to transfer static data like vert buffers is not 
		// recommended. Every time the GPU needs it, the upload heap will be marshalled 
		// over. Please read up on Default Heap usage. An upload heap is used here for 
		// code simplicity and because there are very few verts to actually transfer.
		ThrowIfFailed(D3D12RHI::GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_vertexBuffer)));

		// Copy the triangle data to the vertex buffer.
		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
		ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));

		memcpy(pVertexDataBegin, mesh, sizeof(OGLVertex)*m_numtriangles*3);
		m_vertexBuffer->Unmap(0, nullptr);

		// Initialize the vertex buffer view.
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		m_vertexBufferView.StrideInBytes = sizeof(OGLVertex);
		m_vertexBufferView.SizeInBytes = vertexBufferSize;
	}
}