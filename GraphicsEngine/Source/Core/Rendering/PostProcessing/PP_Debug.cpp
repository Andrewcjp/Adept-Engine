#include "PP_Debug.h"
#include "Core/BaseWindow.h"
#include "Rendering/Core/FrameBuffer.h"
#include "Rendering/Core/LightCulling/LightCullingEngine.h"
#include "Rendering/Core/SceneRenderer.h"
#include "Rendering/Shaders/Culling/Shader_ShowLightDensity.h"

static ConsoleVariable ShowCulling("lc.ShowHeatMap", 0);


PP_Debug::PP_Debug()
{}


PP_Debug::~PP_Debug()
{}

void PP_Debug::ExecPass(RHICommandList * list, FrameBuffer * InputTexture)
{
	if (!ShowCulling.GetBoolValue())
	{
		return;
	}
	RHIPipeLineStateDesc desc = RHIPipeLineStateDesc::CreateDefault(ShaderComplier::GetShader<Shader_ShowLightDensity>());

	list->SetPipelineStateDesc(desc);
	list->SetUAV(InputTexture, "SrcTex");
	list->SetUAV(InputTexture, "DstTexture");
	list->SetRHIBufferReadOnly(SceneRenderer::Get()->GetLightCullingEngine()->LightCullingBuffer, "LightList");
	SceneRenderer::Get()->BindLightsBufferB(list, desc.ShaderInUse->GetSlotForName("LightBuffer"));
	list->Dispatch(LightCullingEngine::GetLightGridDim().x, LightCullingEngine::GetLightGridDim().y, 1);
	list->UAVBarrier(InputTexture);
}

void PP_Debug::PostSetUpData()
{

}

void PP_Debug::PostInitEffect(FrameBuffer* Target)
{

}
