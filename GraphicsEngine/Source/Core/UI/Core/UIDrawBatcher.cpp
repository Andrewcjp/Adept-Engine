#include "UIDrawBatcher.h"
#include "UI/UIManager.h"
#include "Rendering/Shaders/UI/Shader_UIBatch.h"
#include "Core/Utils/StringUtil.h"
#include "RHI/RHI_inc.h"
#include "Core/Platform/PlatformCore.h"
#include "RHI/RHITypes.h"
#include "Rendering/Renderers/TextRenderer.h"
#include "Core/Performance/PerfManager.h"

UIDrawBatcher::UIDrawBatcher()
{
	Shader = ShaderComplier::GetShader<Shader_UIBatch>();
	Init();
	Current_Max_Verts = UIMin;
	PerfManager::Get()->AddTimer("UI Batches", "UI");
}

void UIDrawBatcher::Init()
{
	commandlist = RHI::CreateCommandList();
	VertexBuffer = RHI::CreateRHIBuffer(ERHIBufferType::Vertex);
	const int vertexBufferSize = sizeof(UIVertex) * Current_Max_Verts;
	VertexBuffer->CreateVertexBuffer(sizeof(UIVertex), vertexBufferSize, EBufferAccessType::Dynamic);
}

void UIDrawBatcher::SetState(RHICommandList* list)
{
	RHIPipeLineStateDesc desc;
	desc.InitOLD(false, false, false);
	desc.ShaderInUse = Shader;
	desc.RenderTargetDesc = RHIPipeRenderTargetDesc::GetDefault();
	list->SetPipelineStateDesc(desc);
	Shader->PushTOGPU(list);
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
	BatchedVerts.clear();
	EnqueueSafeRHIRelease(VertexBuffer);
	EnqueueSafeRHIRelease(commandlist);
}

void UIRenderBatch::AddVertex(glm::vec2 vpos, bool Back, glm::vec3 frontcol, glm::vec3 backcol)
{
	UIVertex vert;
	vert.position = vpos;
	vert.UseBackgound = Back;
	vert.FrontColour = frontcol;
	vert.BackColour = backcol;
	Verts.push_back(vert);
}

void UIRenderBatch::AddText(std::string itext, glm::vec2 ipos)
{
	BatchType = ERenderBatchType::Text;
	text = itext;
	pos = ipos;
}

void UIDrawBatcher::ClearVertArray()
{
	BatchedVerts.clear();
	MemoryUtils::DeleteVector(Batches);
	Batches.clear();
}

void UIDrawBatcher::CleanUp()
{
	ReallocBuffer(std::min((int)BatchedVerts.size(), UIMin) + 10);
}

void UIDrawBatcher::AddBatch(UIRenderBatch* Batch)
{
	Batches.push_back(Batch);
}

void UIDrawBatcher::BuildBatches()
{
	for (int i = 0; i < Batches.size(); i++)
	{
		for (int v = 0; v < Batches[i]->Verts.size(); v++)
		{
			BatchedVerts.push_back(Batches[i]->Verts[v]);
		}
	}
}

void UIDrawBatcher::SendToGPU()
{
	BuildBatches();
	if (BatchedVerts.size() == 0)
	{
		return;
	}
	if (BatchedVerts.size() > Current_Max_Verts)
	{
		ReallocBuffer((int)BatchedVerts.size() + 10);
	}
	for (int i = 0; i < BatchedVerts.size(); i++)
	{
		BatchedVerts[i].position += Offset;
	}
	VertexBuffer->UpdateVertexBuffer(&BatchedVerts[0], sizeof(UIVertex)*BatchedVerts.size());
}

void UIDrawBatcher::RenderBatches()
{
	if (BatchedVerts.size() == 0)
	{
		return;
	}
	SCOPE_CYCLE_COUNTER_GROUP("Draw Batches", "UI");
	glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(UIManager::instance->GetWidth()), 0.0f, static_cast<float>(UIManager::instance->GetHeight()));
	Shader->UpdateUniforms(projection);
	commandlist->ResetList();
	commandlist->GetDevice()->GetTimeManager()->StartTimer(commandlist, EGPUTIMERS::UI);
	Render(commandlist);
	commandlist->GetDevice()->GetTimeManager()->EndTimer(commandlist, EGPUTIMERS::UI);
	commandlist->Execute();
}

void UIDrawBatcher::Render(RHICommandList * list)
{
	PerfManager::Get()->AddToCountTimer("UI Batches", Batches.size());
	uint64 offset = 0;
	for (int i = 0; i < Batches.size(); i++)
	{
		UIRenderBatch* batch = Batches[i];
		if (batch->BatchType == ERenderBatchType::Verts)
		{
			SetState(list);
			commandlist->BeginRenderPass(RHI::GetRenderPassDescForSwapChain());
			list->SetVertexBuffer(VertexBuffer);
			list->DrawPrimitive((int)batch->Verts.size(), 1, offset, 0);
			commandlist->EndRenderPass();
			offset += batch->Verts.size();
		}
		else if (batch->BatchType == ERenderBatchType::Text)
		{
			TextRenderer::instance->RenderDirect(list, batch->text, batch->pos, 0.3, glm::vec3(1, 1, 1));
		}
	}
}
