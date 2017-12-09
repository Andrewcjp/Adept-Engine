#include "stdafx.h"
#include "D3D12Plane.h"
#include "RHI\RHI.h"
#include "Rendering/Core/Triangle.h"
#include "D3D12RHI.h"

D3D12Plane::D3D12Plane(float size)
{
	CreateVertexBuffer();
}

D3D12Plane::~D3D12Plane()
{
}


void D3D12Plane::Render(CommandListDef * list)
{
	if (list == nullptr && RHI::GetType() == RenderSystemD3D12)
	{
		printf("Error Null List\n");
		return;
	}
	list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	list->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	list->DrawInstanced(2 * 3, 1, 0, 0);
}
void D3D12Plane::CreateVertexBuffer()
{
#define tri 0
	// Create the vertex buffer.
	{
#if tri
		float m_aspectRatio = 1.7f;
		//// Define the geometry for a triangle.
		Vertex triangleVertices[] =
		{
			{ { 0.0f, 0.25f * m_aspectRatio, 0.0f },{ 0.5f, 0.0f } },
			{ { 0.25f, -0.25f * m_aspectRatio, 0.0f },{ 1.0f, 1.0f } },
			{ { -0.25f, -0.25f * m_aspectRatio, 0.0f },{ 0.0f, 1.0f } }
		};
		const UINT vertexBufferSize = sizeof(triangleVertices);
		m_numtriangles = 1;
#else
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

		Triangle a;
		a.m_vertices[0].m_position = glm::vec3(xpos, ypos + h, 0);
		a.m_vertices[1].m_position = glm::vec3(xpos, ypos, 0);
		a.m_vertices[2].m_position = glm::vec3(xpos + w, ypos, 0);
		a.m_vertices[0].m_texcoords = glm::vec3(0, 1, 0);
		a.m_vertices[1].m_texcoords = glm::vec3(0, 0, 0);
		a.m_vertices[2].m_texcoords = glm::vec3(1, 0, 0);
		Triangle b;
		b.m_vertices[0].m_position = glm::vec3(xpos, ypos + h, 0);
		b.m_vertices[1].m_position = glm::vec3(xpos + w, ypos, 0);
		b.m_vertices[2].m_position = glm::vec3(xpos + w, ypos + h, 0);
		b.m_vertices[0].m_texcoords = glm::vec3(0, 1, 0);
		b.m_vertices[1].m_texcoords = glm::vec3(1, 0, 0);
		b.m_vertices[2].m_texcoords = glm::vec3(1, 1, 0);

		Triangle tris[]{
			a,b
		};

		Triangle* mesh = nullptr;
		//L"C:\\Users\\AANdr\\Dropbox\\Engine\\Engine\\Repo\\GraphicsEngine\\x64\\asset\\models\\House.obj"
	//	m_numtriangles = importOBJMesh(name, &mesh);
		const UINT vertexBufferSize = sizeof(tris);//(sizeof(float)*(4 * 6) * 2 * 3);
#endif


		// Note: using upload heaps to transfer static data like vert buffers is not 
		// recommended. Every time the GPU needs it, the upload heap will be marshalled 
		// over. Please read up on Default Heap usage. An upload heap is used here for 
		// code simplicity and because there are very few verts to actually transfer.
		ThrowIfFailed(D3D12RHI::Instance->m_device->CreateCommittedResource(
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
#if tri
		memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
#else
		memcpy(pVertexDataBegin, tris, vertexBufferSize);
#endif
		m_vertexBuffer->Unmap(0, nullptr);

		// Initialize the vertex buffer view.
		m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
#if tri
		m_vertexBufferView.StrideInBytes = sizeof(Vertex);
#else

		m_vertexBufferView.StrideInBytes = sizeof(OGLVertex);//four floats per vertex
#endif
		m_vertexBufferView.SizeInBytes = vertexBufferSize;
	}
}
