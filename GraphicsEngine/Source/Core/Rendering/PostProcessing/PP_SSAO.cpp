#include "PP_SSAO.h"
#include "Core/BaseWindow.h"
#include "Rendering/Renderers/RenderEngine.h"
#include "Rendering/Shaders/PostProcess/Shader_SSAO.h"


PP_SSAO::PP_SSAO()
{}


PP_SSAO::~PP_SSAO()
{}

void PP_SSAO::ExecPass(RHICommandList * list, FrameBuffer * InputTexture)
{
	Shader_SSAO* s = ShaderComplier::GetShader<Shader_SSAO>();
	RHIPipeLineStateDesc desc = RHIPipeLineStateDesc::CreateDefault(ShaderComplier::GetShader<Shader_SSAO>());
	list->SetPipelineStateDesc(desc);

	FrameBuffer* GBuffer = nullptr;/// BaseWindow::GetCurrentRenderer()->GetGBuffer();
	list->SetFrameBufferTexture(GBuffer, "PosTex", 0);
	list->SetFrameBufferTexture(GBuffer, "NormalTex", 1);
	list->SetFrameBufferTexture(GBuffer, "DepthTexture", -1);
	s->Bind(list);
	list->SetUAV(SSAOOutput->GetUAV(), "DstTexture");
	list->Dispatch(InputTexture->GetWidth(), InputTexture->GetHeight(), 1);
	list->UAVBarrier(SSAOOutput->GetUAV());


	desc.ShaderInUse = ShaderComplier::GetShader<Shader_SSAO_Merge>();
	list->SetPipelineStateDesc(desc);
	InputTexture->BindUAV(list, 1);
	list->SetFrameBufferTexture(SSAOOutput, 0);

	list->Dispatch(InputTexture->GetWidth(), InputTexture->GetHeight(), 1);
	list->UAVBarrier(InputTexture->GetUAV());

}

void PP_SSAO::PostSetUpData()
{

}

void PP_SSAO::PostInitEffect(FrameBuffer* Target)
{
	EnqueueSafeRHIRelease(SSAOOutput);
	RHIFrameBufferDesc desc;
	desc = RHIFrameBufferDesc::CreateColour(Target->GetWidth(), Target->GetHeight());
	desc.AllowUnorderedAccess = true;
	desc.StartingState = GPU_RESOURCE_STATES::RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	SSAOOutput = RHI::CreateFrameBuffer(RHI::GetDefaultDevice(), desc);
}
