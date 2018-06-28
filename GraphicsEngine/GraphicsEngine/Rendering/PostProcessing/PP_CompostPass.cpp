#include "Stdafx.h"
#include "PP_CompostPass.h"
#include "Rendering/Shaders/Shader_Compost.h"
#include "RHI/RHI.h"
#include "UI/TextRenderer.h"
#include "RHI/DeviceContext.h"
PP_CompostPass::PP_CompostPass()
{}


PP_CompostPass::~PP_CompostPass()
{}
#include "RHI/RenderAPIs/D3D12/D3D12Framebuffer.h"
#include "RHI/RenderAPIs/D3D12/D3D12CommandList.h"
void PP_CompostPass::ExecPass(RHICommandList * list, FrameBuffer * InputTexture)
{
	ID3D12GraphicsCommandList* dlist = ((D3D12CommandList*)list)->GetCommandList();
	D3D12FrameBuffer* buffer = (D3D12FrameBuffer*)InputFramebuffer;
	list->SetScreenBackBufferAsRT();
	if (TextRenderer::instance->RunOnSecondDevice)
	{		
	//	buffer->MakeReadyForRead(dlist);
	}	
	buffer->BindBufferToTexture(dlist, 0, 0,RHI::GetDeviceContext(0));


	RenderScreenQuad(list);
	buffer->MakeReadyForCopy(dlist);

}

void PP_CompostPass::PostSetUpData()
{
	CurrentShader = new Shader_Compost(RHI::GetDeviceContext(0));
	cmdlist = RHI::CreateCommandList(RHI::GetDeviceContext(0));
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
