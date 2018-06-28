#include "Stdafx.h"
#include "PP_ColourCorrect.h"
#include "Rendering/Shaders/Shader_ColourCorrect.h"

PP_ColourCorrect::PP_ColourCorrect()
{}


PP_ColourCorrect::~PP_ColourCorrect()
{}

void PP_ColourCorrect::ExecPass(RHICommandList * list, FrameBuffer * InputTexture)
{
	list->SetScreenBackBufferAsRT();
	list->SetFrameBufferTexture(InputTexture, 0,0);
	RenderScreenQuad(list);
}

void PP_ColourCorrect::PostSetUpData()
{
	CurrentShader = new Shader_ColourCorrect(/*RHI::GetDeviceContext(0)*/);
	cmdlist = RHI::CreateCommandList();
}

void PP_ColourCorrect::PostInitEffect()
{
	cmdlist->SetPipelineState(PipeLineState{ false,false,true });
	cmdlist->CreatePipelineState(CurrentShader);
}
