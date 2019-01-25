
#include "PP_CompostPass.h"
#include "Rendering/Shaders/PostProcess/Shader_Compost.h"
#include "RHI/RHI.h"
#include "Rendering/Renderers/TextRenderer.h"
#include "RHI/DeviceContext.h"
PP_CompostPass::PP_CompostPass()
{}


PP_CompostPass::~PP_CompostPass()
{
	SafeDelete(CurrentShader);
}
void PP_CompostPass::ExecPass(RHICommandList * list, FrameBuffer * InputTexture)
{
	list->SetScreenBackBufferAsRT();
	list->SetFrameBufferTexture(InputFramebuffer, 0);
	RenderScreenQuad(list);
}

void PP_CompostPass::PostSetUpData()
{
	CurrentShader = new Shader_Compost(RHI::GetDeviceContext(0));
	CMDlist = RHI::CreateCommandList(ECommandListType::Graphics, RHI::GetDeviceContext(0));
}

void PP_CompostPass::PostInitEffect(FrameBuffer* Target)
{
	RHIPipeLineStateDesc desc;
	desc.RenderTargetDesc.NumRenderTargets = 1;
	desc.RenderTargetDesc.RTVFormats[0] = eTEXTURE_FORMAT::FORMAT_R8G8B8A8_UNORM;
	desc.RenderTargetDesc.DSVFormat = eTEXTURE_FORMAT::FORMAT_D32_FLOAT;
	desc.InitOLD(false, false, false);
	desc.ShaderInUse = CurrentShader;
	CMDlist->SetPipelineStateDesc(desc);

}

void PP_CompostPass::SetInputFrameBuffer(FrameBuffer * buffer)
{
	InputFramebuffer = buffer;
}
