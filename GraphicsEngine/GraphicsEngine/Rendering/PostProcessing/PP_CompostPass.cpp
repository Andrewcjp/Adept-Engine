#include "Stdafx.h"
#include "PP_CompostPass.h"
#include "../Rendering/Shaders/Shader_Compost.h"
#include "../RHI/RHI.h"
PP_CompostPass::PP_CompostPass()
{}


PP_CompostPass::~PP_CompostPass()
{}

void PP_CompostPass::ExecPass(RHICommandList * list, FrameBuffer * InputTexture)
{
	list->SetScreenBackBufferAsRT();
	list->SetFrameBufferTexture(InputFramebuffer, 0);
	RenderScreenQuad(list);
}

void PP_CompostPass::PostSetUpData()
{
	CurrentShader = new Shader_Compost(RHI::GetDeviceContext(0));
	cmdlist = RHI::CreateCommandList();
}

void PP_CompostPass::PostInitEffect()
{
	cmdlist->SetPipelineState(PipeLineState{ false,false,true });
	cmdlist->CreatePipelineState(CurrentShader);
}

void PP_CompostPass::SetInputFrameBuffer(FrameBuffer * buffer)
{
	InputFramebuffer = buffer;
}
