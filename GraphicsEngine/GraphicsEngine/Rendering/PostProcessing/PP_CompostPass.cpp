#include "Stdafx.h"
#include "PP_CompostPass.h"
#include "../Rendering/Shaders/Shader_Compost.h"
#include "../RHI/RHI.h"
#include "../UI/TextRenderer.h"
PP_CompostPass::PP_CompostPass()
{}


PP_CompostPass::~PP_CompostPass()
{}
#include "../RHI/RenderAPIs/D3D12/D3D12Framebuffer.h"
#include "../RHI/RenderAPIs/D3D12/D3D12CommandList.h"
void PP_CompostPass::ExecPass(RHICommandList * list, FrameBuffer * InputTexture)
{

	D3D12FrameBuffer* buffer = (D3D12FrameBuffer*)InputFramebuffer;
	if (TextRenderer::instance->RunOnSecondDevice)
	{		
		buffer->MakeReadyOnTarget(((D3D12CommandList*)list)->GetCommandList());
	}
	list->SetScreenBackBufferAsRT();
	//list->SetFrameBufferTexture(InputFramebuffer, 0);

	buffer->BindBufferToTexture(((D3D12CommandList*)list)->GetCommandList(), 0, 0,RHI::GetDeviceContext(0));


	RenderScreenQuad(list);
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
