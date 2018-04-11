#include "stdafx.h"
#include "UIDrawBatcher.h"
#include "UIManager.h"
#include "../Rendering/Shaders/Shader_UIBatch.h"
#include "../D3D12/D3D12RHI.h"
#include "../D3D12/D3D12Shader.h"
#include "../Core/Utils/StringUtil.h"
#include <algorithm>
UIDrawBatcher* UIDrawBatcher::instance = nullptr;
UIDrawBatcher::UIDrawBatcher()
{
	instance = this;
	Shader = new Shader_UIBatch();
	InitD3D12();
}

void UIDrawBatcher::InitD3D12()
{
	commandlist = RHI::CreateCommandList();
	VertexBuffer = RHI::CreateRHIBuffer(RHIBuffer::Vertex);
	const UINT vertexBufferSize = sizeof(UIVertex) * 500;//mazsize
	VertexBuffer->CreateVertexBuffer(sizeof(UIVertex), vertexBufferSize, RHIBuffer::BufferAccessType::Dynamic);
	commandlist->SetPipelineState(PipeLineState{ false , false });
	commandlist->CreatePipelineState(Shader);
}
void UIDrawBatcher::ReallocBuffer(int NewSize)
{
	//todo: ReallocBuffer
}
void UIDrawBatcher::SendToGPU_D3D12()
{
	VertexBuffer->UpdateVertexBuffer(&BatchedVerts[0], sizeof(UIVertex)*BatchedVerts.size());
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
	SendToGPU_D3D12();
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
		RenderBatches_D3D12();
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
	commandlist->ResetList();
	commandlist->SetScreenBackBufferAsRT();
	Shader->PushTOGPU(commandlist);
	Render(commandlist);
	commandlist->Execute();
}
void UIDrawBatcher::Render(RHICommandList * list)
{
	list->SetVertexBuffer(VertexBuffer);
	list->DrawPrimitive((int)BatchedVerts.size(), 1, 0, 0);
}
