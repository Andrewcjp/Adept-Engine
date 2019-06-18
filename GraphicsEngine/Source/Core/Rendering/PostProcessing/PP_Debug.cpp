#include "PP_Debug.h"
#include "Rendering/Core/FrameBuffer.h"
#include "Rendering/Shaders/Culling/Shader_ShowLightDensity.h"
#include "Core/BaseWindow.h"
#include "../Renderers/RenderEngine.h"
#include "../Core/LightCulling/LightCullingEngine.h"
#include "../Core/SceneRenderer.h"



PP_Debug::PP_Debug()
{}


PP_Debug::~PP_Debug()
{}

void PP_Debug::ExecPass(RHICommandList * list, FrameBuffer * InputTexture)
{
	RHIPipeLineStateDesc desc = RHIPipeLineStateDesc::CreateDefault(ShaderComplier::GetShader<Shader_ShowLightDensity>());

	list->SetPipelineStateDesc(desc);
	list->SetUAV(InputTexture->GetUAV(), "SrcTex");
	list->SetUAV(InputTexture->GetUAV(), "DstTexture");
	list->SetRHIBufferReadOnly(BaseWindow::GetCurrentRenderer()->LightCulling->LightCullingBuffer, "LightList");
	BaseWindow::GetCurrentRenderer()->SceneRender->BindLightsBuffer(list, desc.ShaderInUse->GetSlotForName("LightBuffer"));
	list->Dispatch(LightCullingEngine::GetLightGridDim().x, LightCullingEngine::GetLightGridDim().y, 1);
	list->UAVBarrier(InputTexture->GetUAV());
}

void PP_Debug::PostSetUpData()
{

}

void PP_Debug::PostInitEffect(FrameBuffer* Target)
{

}
