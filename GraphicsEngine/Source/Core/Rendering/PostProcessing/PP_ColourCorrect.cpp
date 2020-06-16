
#include "PP_ColourCorrect.h"
#include "Rendering/Shaders/PostProcess/Shader_ColourCorrect.h"

#include "RHI/DeviceContext.h"
#include "RHI/RHI_inc.h"
#include "Editor/EditorWindow.h"
#include "Core/EngineTypes.h"
#include "../Core/FrameBuffer.h"
PP_ColourCorrect::PP_ColourCorrect()
{}


PP_ColourCorrect::~PP_ColourCorrect()
{

}

void PP_ColourCorrect::ExecPass(RHICommandList * list, FrameBuffer * InputTexture)
{
	//list->SetScreenBackBufferAsRT();
	//list->ClearScreen();
	list->SetFrameBufferTexture(InputTexture, 0);
	if (AddtiveBuffer != nullptr)
	{
		list->SetFrameBufferTexture(AddtiveBuffer, 1);
	}
	else
	{
		list->SetFrameBufferTexture(nullptr, 1);
	}
#if WITH_EDITOR
	if (EditorWindow::GetInstance()->UseSmallerViewPort())
	{
		IntRect rect = EditorWindow::GetInstance()->GetViewPortRect();
		list->SetViewport(rect.Min.x, rect.Min.y, rect.Max.x, rect.Max.y, 0, 0);
	}
#endif
	RenderScreenQuad(list);
	InputTexture->MakeReadyForComputeUse(list);
	AddtiveBuffer->MakeReadyForComputeUse(list);
}

void PP_ColourCorrect::PostSetUpData()
{
	CurrentShader = ShaderCompiler::GetShader<Shader_ColourCorrect>();
//	CMDlist = RHI::CreateCommandList();
}

void PP_ColourCorrect::PostInitEffect(FrameBuffer* Target)
{
	//PipeLineState state = PipeLineState{ false,false,true };
	//state.RenderTargetDesc.NumRenderTargets = 1;
	//state.RenderTargetDesc.RTVFormats[0] = eTEXTURE_FORMAT::FORMAT_R8G8B8A8_UNORM;
	//state.RenderTargetDesc.DSVFormat = eTEXTURE_FORMAT::FORMAT_D32_FLOAT;
	//CMDlist->SetPipelineState_OLD(state);
	//CMDlist->CreatePipelineState(CurrentShader);
	RHIPipeLineStateDesc state;
	state.ShaderInUse = CurrentShader;
	state.Cull = false;
	state.DepthStencilState.DepthEnable = false;
	state.RenderTargetDesc.NumRenderTargets = 1;
	state.RenderTargetDesc.RTVFormats[0] = ETextureFormat::FORMAT_R8G8B8A8_UNORM;
	state.RenderTargetDesc.DSVFormat = ETextureFormat::FORMAT_D32_FLOAT;
	//CMDlist->SetPipelineStateDesc(state);

}
