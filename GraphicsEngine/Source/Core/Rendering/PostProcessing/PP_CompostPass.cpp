
#include "PP_CompostPass.h"
#include "Rendering/Shaders/PostProcess/Shader_Compost.h"
#include "RHI/RHI.h"
#include "Rendering/Renderers/TextRenderer.h"
#include "RHI/DeviceContext.h"
PP_CompostPass::PP_CompostPass()
{}


PP_CompostPass::~PP_CompostPass()
{
	delete CurrentShader;
}
#define TEST 0
#if TEST
#include "RHI/RenderAPIs/D3D12/D3D12Framebuffer.h"
#include "RHI/RenderAPIs/D3D12/D3D12CommandList.h"
#include "RHI/RenderAPIs/D3D12/GPUResource.h"
#endif
void PP_CompostPass::ExecPass(RHICommandList * list, FrameBuffer * InputTexture)
{
#if TEST
	ID3D12GraphicsCommandList* dlist = ((D3D12CommandList*)list)->GetCommandList();
	D3D12FrameBuffer* buffer = (D3D12FrameBuffer*)InputFramebuffer;
#endif
	list->SetScreenBackBufferAsRT();
	if (TextRenderer::instance->RunOnSecondDevice)
	{
		//	buffer->MakeReadyForRead(dlist);
	}
	list->SetFrameBufferTexture(InputFramebuffer, 0);

	RenderScreenQuad(list);
#if TEST
	if (TextRenderer::instance->RunOnSecondDevice)
	{
		buffer->MakeReadyForCopy(dlist);
	}
	if (buffer->GetDescription().AllowUnordedAccess)
	{
		buffer->GetResource(0)->SetResourceState(dlist, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}
#endif
}

void PP_CompostPass::PostSetUpData()
{
	CurrentShader = new Shader_Compost(RHI::GetDeviceContext(0));
	CMDlist = RHI::CreateCommandList(ECommandListType::Graphics, RHI::GetDeviceContext(0));
}

void PP_CompostPass::PostInitEffect(FrameBuffer* Target)
{
	PipeLineState state = PipeLineState{ false,false,true };
	state.RenderTargetDesc.NumRenderTargets = 1;
	state.RenderTargetDesc.RTVFormats[0] = eTEXTURE_FORMAT::FORMAT_R8G8B8A8_UNORM;
	state.RenderTargetDesc.DSVFormat = eTEXTURE_FORMAT::FORMAT_D32_FLOAT;
	CMDlist->SetPipelineState_OLD(state);
	CMDlist->CreatePipelineState(CurrentShader);
}

void PP_CompostPass::SetInputFrameBuffer(FrameBuffer * buffer)
{
	InputFramebuffer = buffer;
}
