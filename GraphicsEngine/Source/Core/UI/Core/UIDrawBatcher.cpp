#include "stdafx.h"
#include <algorithm>
#include "UIDrawBatcher.h"
#include "UI/UIManager.h"
#include "Rendering/Shaders/Shader_UIBatch.h"
#include "Core/Utils/StringUtil.h"
#include "RHI/RHI_inc.h"

UIDrawBatcher* UIDrawBatcher::instance = nullptr;
UIDrawBatcher::UIDrawBatcher()
{
	instance = this;
	Shader = new Shader_UIBatch();
	Init();
}

void UIDrawBatcher::Init()
{
	commandlist = RHI::CreateCommandList();
	VertexBuffer = RHI::CreateRHIBuffer(RHIBuffer::Vertex);
	const UINT vertexBufferSize = sizeof(UIVertex) * Max_Verts;
	VertexBuffer->CreateVertexBuffer(sizeof(UIVertex), vertexBufferSize, RHIBuffer::BufferAccessType::Dynamic);
	commandlist->SetPipelineState(PipeLineState{ false , false });
	commandlist->CreatePipelineState(Shader);	
}
void UIDrawBatcher::ReallocBuffer(int NewSize)
{
	//todo: ReallocBuffer
}

UIDrawBatcher::~UIDrawBatcher()
{
	BatchedVerts.empty();
	delete Shader;
	delete VertexBuffer;
	delete commandlist;
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
	VertexBuffer->UpdateVertexBuffer(&BatchedVerts[0], sizeof(UIVertex)*BatchedVerts.size());
}

void UIDrawBatcher::RenderBatches()
{
	if (BatchedVerts.size() == 0)
	{
		return;
	}
	ensure(BatchedVerts.size() < Max_Verts);
	Shader->SetShaderActive();
	glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(UIManager::instance->GetWidth()), 0.0f, static_cast<float>(UIManager::instance->GetHeight()));
	Shader->UpdateUniforms(projection);

	commandlist->ResetList();
	commandlist->GetDevice()->GetTimeManager()->StartTimer(commandlist, EGPUTIMERS::UI);
	commandlist->SetScreenBackBufferAsRT();
	Shader->PushTOGPU(commandlist);
	Render(commandlist);
	commandlist->GetDevice()->GetTimeManager()->EndTimer(commandlist, EGPUTIMERS::UI);
	commandlist->Execute();	
}

void UIDrawBatcher::Render(RHICommandList * list)
{
	list->SetVertexBuffer(VertexBuffer);
	list->DrawPrimitive((int)BatchedVerts.size(), 1, 0, 0);
}
