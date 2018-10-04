#include "stdafx.h"
#include <algorithm>
#include "UIDrawBatcher.h"
#include "UI/UIManager.h"
#include "Rendering/Shaders/Shader_UIBatch.h"
#include "Core/Utils/StringUtil.h"
#include "RHI/RHI_inc.h"
#include "Core/Platform/PlatformCore.h"
#include "RHI/RHITypes.h"
UIDrawBatcher* UIDrawBatcher::instance = nullptr;
UIDrawBatcher::UIDrawBatcher()
{
	instance = this;
	Shader = ShaderComplier::GetShader<Shader_UIBatch>();
	Init();
	Current_Max_Verts = UIMin;
}

void UIDrawBatcher::Init()
{
	commandlist = RHI::CreateCommandList();
	VertexBuffer = RHI::CreateRHIBuffer(ERHIBufferType::Vertex);
	const int vertexBufferSize = sizeof(UIVertex) * Current_Max_Verts;
	VertexBuffer->CreateVertexBuffer(sizeof(UIVertex), vertexBufferSize, EBufferAccessType::Dynamic);
	commandlist->SetPipelineState(PipeLineState{ false , false });
	commandlist->CreatePipelineState(Shader);
}
void UIDrawBatcher::ReallocBuffer(int NewSize)
{
	ensure(NewSize < Max_Verts);
	EnqueueSafeRHIRelease(VertexBuffer);
	Current_Max_Verts = NewSize;
	VertexBuffer = RHI::CreateRHIBuffer(ERHIBufferType::Vertex);
	const int vertexBufferSize = sizeof(UIVertex) * Current_Max_Verts;
	VertexBuffer->CreateVertexBuffer(sizeof(UIVertex), vertexBufferSize, EBufferAccessType::Dynamic);
}

UIDrawBatcher::~UIDrawBatcher()
{
	BatchedVerts.empty();
	EnqueueSafeRHIRelease(VertexBuffer);
	EnqueueSafeRHIRelease(commandlist);
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
	if (BatchedVerts.size() > Current_Max_Verts)
	{
		ReallocBuffer((int)BatchedVerts.size() + 10);
	}
	VertexBuffer->UpdateVertexBuffer(&BatchedVerts[0], sizeof(UIVertex)*BatchedVerts.size());
}

void UIDrawBatcher::RenderBatches()
{
	if (BatchedVerts.size() == 0)
	{
		return;
	}
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
