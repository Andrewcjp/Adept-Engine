#include "stdafx.h"
#include "D3D12Plane.h"
#include "D3D12RHI.h"

D3D12Plane::D3D12Plane(float size)
{
	CreateVertexBuffer();
}

D3D12Plane::~D3D12Plane()
{
}


//void D3D12Plane::Render(RHICommandList * list)
//{
//	if (list == nullptr && RHI::GetType() == RenderSystemD3D12)
//	{
//		printf("Error Null List\n");
//		return;
//	}
//	//list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//	//list->IASetVertexBuffers(0, 1, &m_vertexBufferView);
//	//list->DrawInstanced(2 * 3, 1, 0, 0);
//}
void D3D12Plane::CreateVertexBuffer()
{

	// Create the vertex buffer.
	{

		float h = 5;
		float w = 5;
		float xoff = -5;


		float xpos = -(w / 2.0f) + xoff;
		float ypos = -(h / 2.0f) + 0.1f;



		static const float g_quad_vertex_buffer_data[] = {
			xpos,     ypos + h,   0.0 ,0.0 ,
			xpos,     ypos,      0.0 ,0.0 ,
			xpos + w, ypos,       0.0 ,0.0 ,

			xpos,     ypos + h,   0.0 ,0.0 ,
			xpos + w, ypos,       0.0 ,0.0 ,
			xpos + w , ypos + h,  0.0 ,0.0 ,
		};
		OGLVertex m_vertices[6];

		
		m_vertices[0].m_position = glm::vec3(xpos, ypos + h, 0);
		m_vertices[1].m_position = glm::vec3(xpos, ypos, 0);
		m_vertices[2].m_position = glm::vec3(xpos + w, ypos, 0);
		m_vertices[0].m_texcoords = glm::vec2(0, 1);
		m_vertices[1].m_texcoords = glm::vec2(0, 0);
		m_vertices[2].m_texcoords = glm::vec2(1, 0);
	
		m_vertices[3].m_position = glm::vec3(xpos, ypos + h, 0);
		m_vertices[4].m_position = glm::vec3(xpos + w, ypos, 0);
		m_vertices[5].m_position = glm::vec3(xpos + w, ypos + h, 0);
		m_vertices[3].m_texcoords = glm::vec2(0, 1);
		m_vertices[4].m_texcoords = glm::vec2(1, 0);
		m_vertices[5].m_texcoords = glm::vec2(1, 1);

		
		const UINT vertexBufferSize = sizeof(m_vertices);

		// Note: using upload heaps to transfer static data like vert buffers is not 
		// recommended. Every time the GPU needs it, the upload heap will be marshalled 
		// over. Please read up on Default Heap usage. An upload heap is used here for 
		// code simplicity and because there are very few verts to actually transfer.
	/*	ThrowIfFailed(D3D12RHI::GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_vertexBuffer)));*/

		// Copy the triangle data to the vertex buffer.
		UINT8* pVertexDataBegin;
		CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
		ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));

		memcpy(pVertexDataBegin, m_vertices, vertexBufferSize);
		m_vertexBuffer->Unmap(0, nullptr);

		// Initialize the vertex buffer view.
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
		
		m_vertexBufferView.StrideInBytes = sizeof(OGLVertex);//four floats per vertex
		m_vertexBufferView.SizeInBytes = vertexBufferSize;
	}
}
