#include "stdafx.h"
#include "PostProcessEffectBase.h"
#include "../RHI/RHI.h"

PostProcessEffectBase::PostProcessEffectBase()
{}
void PostProcessEffectBase::SetUpData()
{
	float g_quad_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,0.0f,
		1.0f, -1.0f, 0.0f,0.0f,
		-1.0f,  1.0f, 0.0f,0.0f,
		-1.0f,  1.0f, 0.0f,0.0f,
		1.0f, -1.0f, 0.0f,0.0f,
		1.0f,  1.0f, 0.0f,0.0f,
	};
	VertexBuffer = RHI::CreateRHIBuffer(RHIBuffer::BufferType::Vertex);
	VertexBuffer->CreateVertexBuffer(sizeof(float)*4, sizeof(float)*6*4);
	VertexBuffer->UpdateVertexBuffer(&g_quad_vertex_buffer_data, sizeof(float) * 6 * 4);

	//test

	CurrentShader = new ShaderOutput(10,1);
	cmdlist = RHI::CreateCommandList();

}
void PostProcessEffectBase::TestOutput()
{

}

PostProcessEffectBase::~PostProcessEffectBase()
{}

void PostProcessEffectBase::InitEffect()
{
	cmdlist->SetPipelineState(PipeLineState{ false,false });
	cmdlist->CreatePipelineState(CurrentShader);
}
void PostProcessEffectBase::RunPass(RHICommandList * list,FrameBuffer* InputTexture)
{
	list->ResetList();
	
	list->SetScreenBackBufferAsRT();
	list->SetFrameBufferTexture(InputTexture, 0);
	RenderScreenQuad(list);
	list->Execute();
}

void PostProcessEffectBase::RenderScreenQuad(RHICommandList * list)
{
	//todo: less than full screen!
	list->SetVertexBuffer(VertexBuffer);
	list->DrawPrimitive(6, 1, 0, 0);

}

