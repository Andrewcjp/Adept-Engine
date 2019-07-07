#include "ReflectionEnviroment.h"
#include "RelfectionProbe.h"
#include "Core\Performance\PerfManager.h"
#include "Material.h"
#include "..\Shaders\Shader_Main.h"
#include "SceneRenderer.h"
#include "..\Shaders\Shader_Skybox.h"


ReflectionEnviroment::ReflectionEnviroment()
{}


ReflectionEnviroment::~ReflectionEnviroment()
{}

void ReflectionEnviroment::Update()
{}


void ReflectionEnviroment::UpdateRelflectionProbes(RHICommandList* commandlist)
{
	SCOPE_CYCLE_COUNTER_GROUP("Update Relflection Probes", "Render");
	commandlist->StartTimer(EGPUTIMERS::CubemapCapture);
	for (int i = 0; i < Probes.size(); i++)
	{
		RelfectionProbe* Probe = Probes[i];
		RenderCubemap(Probe, commandlist);
	}
	commandlist->EndTimer(EGPUTIMERS::CubemapCapture);
}

bool ReflectionEnviroment::AnyProbesNeedUpdate()
{
	for (int i = 0; i < Probes.size(); i++)
	{
		if (Probes[i]->NeedsCapture())
		{
			return true;
		}
	}
	return false;
}

void ReflectionEnviroment::RenderCubemap(RelfectionProbe * Map, RHICommandList* commandlist)
{
	if (!Map->NeedsCapture())
	{
		return;
	}
	//	commandlist->ClearFrameBuffer(Map->CapturedTexture);
	RHIPipeLineStateDesc Desc = RHIPipeLineStateDesc::CreateDefault(Material::GetDefaultMaterialShader(), Map->CapturedTexture);
	for (int i = 0; i < 6; i++)
	{
		commandlist->SetPipelineStateDesc(Desc);
		SceneRenderer::Get()->SetMVForProbe(commandlist, i, MainShaderRSBinds::MVCBV);
		RHIRenderPassDesc Info;
		Info.TargetBuffer = Map->CapturedTexture;
		Info.LoadOp = ERenderPassLoadOp::Clear;
		commandlist->BeginRenderPass(Info);
		//commandlist->SetRenderTarget(Map->CapturedTexture, i);
		SceneRenderer::Get()->RenderScene(commandlist, false, Map->CapturedTexture, true);
		commandlist->EndRenderPass();
		SceneRenderer::Get()->SB->Render(SceneRenderer::Get(), commandlist, Map->CapturedTexture, nullptr, true, i);

	}
	Map->SetCaptured();
	//FrameBufferProcessor::CreateMipChain(Map->CapturedTexture, commandlist);
}