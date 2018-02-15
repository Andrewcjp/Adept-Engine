#include "stdafx.h"
#include "UIDrawBatcher.h"
#include "UIManager.h"
#include "../Rendering/Shaders/Shader_UIBatch.h"
#include "../D3D12/D3D12RHI.h"
#include "../D3D12/D3D12Shader.h"
#include "../Core/Utils/StringUtil.h"
UIDrawBatcher* UIDrawBatcher::instance = nullptr;
UIDrawBatcher::UIDrawBatcher()
{
	instance = this;
	Shader = new Shader_UIBatch();
	if (RHI::IsOpenGL())
	{
		InitOGL();
	}
	else if (RHI::IsD3D12())
	{
		InitD3D12();
	}
}
void UIDrawBatcher::InitOGL()
{
	glGenBuffers(1, &quad_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
}
void UIDrawBatcher::InitD3D12()
{
	// Create the vertex buffer.


	const UINT vertexBufferSize = sizeof(UIVertex) * 500;//mazsize

	// Note: using upload heaps to transfer static data like vert buffers is not 
	// recommended. Every time the GPU needs it, the upload heap will be marshalled 
	// over. Please read up on Default Heap usage. An upload heap is used here for 
	// code simplicity and because there are very few verts to actually transfer.
	ThrowIfFailed(D3D12RHI::Instance->m_Primarydevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_vertexBuffer)));

	// Initialize the vertex buffer view.
	m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(UIVertex);
	m_vertexBufferView.SizeInBytes = vertexBufferSize;
	m_vertexBuffer->SetName(StringUtils::ConvertStringToWide("UI Vertex Buffer").c_str());
	UIList = Shader->GetD3D12Shader()->CreateShaderCommandList();
	UIList->SetName(StringUtils::ConvertStringToWide("UIList").c_str());

}
void UIDrawBatcher::ReallocBuffer(int NewSize)
{
	//todo: ReallocBuffer
}
void UIDrawBatcher::SendToGPU_D3D12()
{
	// Copy the triangle data to the vertex buffer.
	UINT8* pVertexDataBegin;
	CD3DX12_RANGE readRange(0, 0);		// We do not intend to read from this resource on the CPU.
	ThrowIfFailed(m_vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));

	memcpy(pVertexDataBegin, &BatchedVerts[0], sizeof(UIVertex)*BatchedVerts.size());
	m_vertexBuffer->Unmap(0, nullptr);


}
UIDrawBatcher::~UIDrawBatcher()
{
	BatchedVerts.empty();
	delete Shader;
}
void UIDrawBatcher::AddVertex(glm::vec2 pos, bool Back, glm::vec3 frontcol, glm::vec3 backcol)
{
	UIVertex vert;
	vert.position = pos;
	vert.UseBackgound = Back;
	vert.FrontColour = frontcol;
	vert.BackColour = backcol;
	BatchedVerts.push_back(vert);
}

void UIDrawBatcher::ClearVertArray()
{
	BatchedVerts.clear();
}

void UIDrawBatcher::CleanUp()
{
	ReallocBuffer(std::min((int)BatchedVerts.size(), UIMin) + 10);
}

void UIDrawBatcher::SendToGPU()
{
	if (BatchedVerts.size() == 0)
	{
		return;
	}
	if (RHI::IsOpenGL())
	{
		SendToGPU_OpenGL();
	}
	if (RHI::IsD3D12())
	{
		SendToGPU_D3D12();
	}
}
void UIDrawBatcher::SendToGPU_OpenGL()
{
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(UIVertex)*BatchedVerts.size(), &BatchedVerts[0], GL_DYNAMIC_DRAW);
}


void UIDrawBatcher::RenderBatches()
{
	if (BatchedVerts.size() == 0)
	{
		return;
	}
	Shader->SetShaderActive();
	glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(UIManager::instance->GetWidth()), 0.0f, static_cast<GLfloat>(UIManager::instance->GetHeight()));
	Shader->UpdateUniforms(projection);
	if (RHI::IsOpenGL())
	{
		RenderBatches_OpenGL();
	}
	else if (RHI::IsD3D12())
	{
		if (D3D12RHI::Instance->HasSetup)
		{
			RenderBatches_D3D12();
		}
	}
}
void UIDrawBatcher::RenderBatches_D3D12()
{
	Shader->SetShaderActive();
	Shader->GetD3D12Shader()->ResetList(UIList);
	D3D12RHI::Instance->SetScreenRenderTaget(UIList);
	D3D12RHI::Instance->RenderToScreen(UIList);
	Shader->PushTOGPU(UIList);
	Render(UIList);
	D3D12RHI::Instance->ExecList(UIList);

}
void UIDrawBatcher::Render(CommandListDef * list)
{
	if (list == nullptr && RHI::GetType() == RenderSystemD3D12)
	{
		printf("Error Null List\n");
		return;
	}
	list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	list->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	list->DrawInstanced((int)BatchedVerts.size(), 1, 0, 0);
	list->Close();
}
void UIDrawBatcher::RenderBatches_OpenGL()
{
	glVertexAttribDivisor(0, 0);
	glVertexAttribDivisor(1, 0);
	glVertexAttribDivisor(2, 0);
	glDisable(GL_BLEND);


	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	GLsizei size = (sizeof(UIVertex));
	glVertexAttribDivisor(1, 0);//particles did not clear 
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		2,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		size,                  // stride
		(void*)0            // array buffer offset
	);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		1,                  // size
		GL_INT,           // type
		GL_FALSE,           // normalized?
		size,                  // stride
		(void*)(sizeof(GLfloat) * 2)            // array buffer offset
	);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		size,                  // stride
		(void*)((sizeof(GLfloat) * 2) + sizeof(GLint))            // array buffer offset
	);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(
		3,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		size,                  // stride
		(void*)(0 + sizeof(glm::vec2) + sizeof(GLint) + sizeof(glm::vec3))          // array buffer offset
	);
	// Draw the triangles !
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)BatchedVerts.size()); // 2*3 indices starting at 0 -> 2 triangles

	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
}