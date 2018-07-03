#include "Stdafx.h"
#include "PP_CompostPass.h"
#include "Rendering/Shaders/PostProcess/Shader_Compost.h"
#include "RHI/RHI.h"
#include "Rendering/Renderers/TextRenderer.h"
#include "RHI/DeviceContext.h"
PP_CompostPass::PP_CompostPass()
{}


PP_CompostPass::~PP_CompostPass()
{}
#include "RHI/RenderAPIs/D3D12/D3D12Framebuffer.h"
#include "RHI/RenderAPIs/D3D12/D3D12CommandList.h"
#include "../RHI/RenderAPIs/D3D12/GPUResource.h"
void PP_CompostPass::ExecPass(RHICommandList * list, FrameBuffer * InputTexture)
{
	ID3D12GraphicsCommandList* dlist = ((D3D12CommandList*)list)->GetCommandList();
	D3D12FrameBuffer* buffer = (D3D12FrameBuffer*)InputFramebuffer;
	list->SetScreenBackBufferAsRT();
	if (TextRenderer::instance->RunOnSecondDevice)
	{
		//	buffer->MakeReadyForRead(dlist);
	}
	buffer->BindBufferToTexture(dlist, 0, 0, RHI::GetDeviceContext(0));


	RenderScreenQuad(list);
	if (TextRenderer::instance->RunOnSecondDevice)
	{
		buffer->MakeReadyForCopy(dlist);
	}
	if (buffer->GetDescription().AllowUnordedAccess)
	{
		buffer->GetResource(0)->SetResourceState(dlist, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}
}

void PP_CompostPass::PostSetUpData()
{
	CurrentShader = new Shader_Compost(RHI::GetDeviceContext(0));
	CMDlist = RHI::CreateCommandList(ECommandListType::Graphics,RHI::GetDeviceContext(0));
}

void PP_CompostPass::PostInitEffect(FrameBuffer* Target)
{
	CMDlist->SetPipelineState(PipeLineState{ false,false,true });
	CMDlist->CreatePipelineState(CurrentShader);
}

void PP_CompostPass::SetInputFrameBuffer(FrameBuffer * buffer)
{
	InputFramebuffer = buffer;
}
