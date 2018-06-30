#include "Stdafx.h"
#include "PP_ColourCorrect.h"
#include "Rendering/Shaders/Shader_ColourCorrect.h"
#include "../RHI/RenderAPIs/D3D12/D3D12CommandList.h"
#include "../RHI/RenderAPIs/D3D12/D3D12Framebuffer.h"
#include "../RHI/DeviceContext.h"
#include "../RHI/RenderAPIs/D3D12/GPUResource.h"

PP_ColourCorrect::PP_ColourCorrect()
{}


PP_ColourCorrect::~PP_ColourCorrect()
{}

void PP_ColourCorrect::ExecPass(RHICommandList * list, FrameBuffer * InputTexture)
{
	list->SetScreenBackBufferAsRT();
	list->SetFrameBufferTexture(InputTexture, 0,0);
	RenderScreenQuad(list);
	D3D12FrameBuffer* dBuffer = (D3D12FrameBuffer*)InputTexture;
	dBuffer->GetResource(0)->SetResourceState(((D3D12CommandList*)list)->GetCommandList(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

void PP_ColourCorrect::PostSetUpData()
{
	CurrentShader = new Shader_ColourCorrect(/*RHI::GetDeviceContext(0)*/);
	CMDlist = RHI::CreateCommandList();
}

void PP_ColourCorrect::PostInitEffect(FrameBuffer* Target)
{
	CMDlist->SetPipelineState(PipeLineState{ false,false,true });
	CMDlist->CreatePipelineState(CurrentShader);
}
