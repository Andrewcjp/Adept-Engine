#include "Stdafx.h"
#include "PP_ColourCorrect.h"
#include "Rendering/Shaders/PostProcess/Shader_ColourCorrect.h"

#include "RHI/DeviceContext.h"
#include "RHI/RHI_inc.h"

PP_ColourCorrect::PP_ColourCorrect()
{}


PP_ColourCorrect::~PP_ColourCorrect()
{
	delete CurrentShader;
}

void PP_ColourCorrect::ExecPass(RHICommandList * list, FrameBuffer * InputTexture)
{
	list->SetScreenBackBufferAsRT();
	list->ClearScreen();
	list->SetFrameBufferTexture(InputTexture, 0);
	if (AddtiveBuffer != nullptr)
	{
		list->SetFrameBufferTexture(AddtiveBuffer, 1);
	}
	else
	{
		list->SetFrameBufferTexture(nullptr, 1);
	}

	RenderScreenQuad(list);
#if 0
	D3D12FrameBuffer* dBuffer = (D3D12FrameBuffer*)InputTexture;
	dBuffer->GetResource(0)->SetResourceState(((D3D12CommandList*)list)->GetCommandList(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	dBuffer = (D3D12FrameBuffer*)AddtiveBuffer;
	dBuffer->GetResource(0)->SetResourceState(((D3D12CommandList*)list)->GetCommandList(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
#endif
}

void PP_ColourCorrect::PostSetUpData()
{
	CurrentShader = new Shader_ColourCorrect(/*RHI::GetDeviceContext(0)*/);
	CMDlist = RHI::CreateCommandList();
}

void PP_ColourCorrect::PostInitEffect(FrameBuffer* Target)
{
	PipeLineState state = PipeLineState{ false,false,true };
	state.RenderTargetDesc.NumRenderTargets = 1;
	state.RenderTargetDesc.RTVFormats[0] = eTEXTURE_FORMAT::FORMAT_R8G8B8A8_UNORM;
	state.RenderTargetDesc.DSVFormat = eTEXTURE_FORMAT::FORMAT_D32_FLOAT;
	CMDlist->SetPipelineState(state);
	CMDlist->CreatePipelineState(CurrentShader);
}
