#include "ReflectionEnviroment.h"
#include "ReflectionProbe.h"
#include "Core\Performance\PerfManager.h"
#include "Material.h"
#include "..\Shaders\Shader_Main.h"
#include "SceneRenderer.h"
#include "..\Shaders\Shader_Skybox.h"
#include "Core\Assets\Scene.h"
#include "..\Shaders\Generation\Shader_Convolution.h"
#include "..\Shaders\Generation\Shader_EnvMap.h"
#include "..\Shaders\Shader_Deferred.h"
#include "LightCulling\LightCullingEngine.h"
#include "..\Shaders\Shadow/Shader_Depth.h"
#include "FrameBufferProcessor.h"


ReflectionEnviroment::ReflectionEnviroment()
{
	StaticGenList = RHI::CreateCommandList(ECommandListType::Graphics);
	for (int i = 0; i < RHI::GetDeviceCount(); i++)
	{
		GpuData[i].init(i);
	}

	//Probes.push_back(new ReflectionProbe(glm::vec3(0, 5, 0)));
}


ReflectionEnviroment::~ReflectionEnviroment()
{}

void ReflectionEnviroment::Update()
{}


void ReflectionEnviroment::UpdateRelflectionProbes(RHICommandList* commandlist)
{
	return;
	SCOPE_CYCLE_COUNTER_GROUP("Update Relflection Probes", "Render");
	commandlist->StartTimer(EGPUTIMERS::CubemapCapture);
	BindStaticSceneEnivoment(commandlist, false);
	for (int i = 0; i < Probes.size(); i++)
	{
		ReflectionProbe* Probe = Probes[i];
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

void ReflectionEnviroment::RenderCubemap(ReflectionProbe * Map, RHICommandList* commandlist)
{
	return;
	if (!Map->NeedsCapture())
	{
		return;
	}
	commandlist->ClearFrameBuffer(Map->CapturedTexture);
	RHIPipeLineStateDesc Desc = RHIPipeLineStateDesc::CreateDefault(Material::GetDefaultMaterialShader(), Map->CapturedTexture);
	commandlist->SetPipelineStateDesc(Desc);
	SceneRenderer::Get()->SetupBindsForForwardPass(commandlist, 0, Map->CapturedTexture);
	for (int i = 0; i < CUBE_SIDES; i++)
	{
		commandlist->SetPipelineStateDesc(Desc);
		Map->BindViews(commandlist, i, MainShaderRSBinds::MVCBV);
		RHIRenderPassDesc Info;
		Info.TargetBuffer = Map->CapturedTexture;
		Info.LoadOp = ERenderPassLoadOp::Load;
		Info.SubResourceIndex = i;
		commandlist->BeginRenderPass(Info);
		SceneRenderer::Get()->GetLightCullingEngine()->BindLightBuffer(commandlist);
		MeshPassRenderArgs Args;
		Args.PassType = ERenderPass::BasePass_Cubemap;
		Args.UseDeferredShaders = false;
		SceneRenderer::Get()->MeshController->RenderPass(Args, commandlist);
		commandlist->EndRenderPass();
		//ShaderComplier::GetShader<Shader_Skybox>()->Render(SceneRenderer::Get(), commandlist, Map->CapturedTexture, nullptr, Map, i);
	}
	//Conv->ComputeConvolutionProbe(commandlist, Map->CapturedTexture, Map->ConvolutionBuffer);
	Map->SetCaptured();
	Map->CapturedTexture->MakeReadyForComputeUse(commandlist);
}

void ReflectionEnviroment::DownSampleAndBlurProbes(RHICommandList* ComputeList)
{
	for (int i = 0; i < Probes.size(); i++)
	{
		//FrameBufferProcessor::CreateMipChain(Probes[i]->CapturedTexture, ComputeList);
		FrameBufferProcessor::GenerateBlurChain(Probes[i]->CapturedTexture, ComputeList);
	}
}

void ReflectionEnviroment::BindDynamicReflections(RHICommandList* List, bool IsDeferredshader)
{
	if (IsDeferredshader)
	{
		List->SetFrameBufferTexture(Probes[0]->ConvolutionBuffer, DeferredLightingShaderRSBinds::DiffuseIr);
		List->SetFrameBufferTexture(Probes[0]->CapturedTexture, DeferredLightingShaderRSBinds::SpecBlurMap);
	}
	else
	{
		List->SetFrameBufferTexture(Probes[0]->ConvolutionBuffer, MainShaderRSBinds::DiffuseIr);
		List->SetFrameBufferTexture(Probes[0]->CapturedTexture, MainShaderRSBinds::SpecBlurMap);
	}
}

void ReflectionEnviroment::BindStaticSceneEnivoment(RHICommandList * List, bool IsDeferredshader)
{
	if (IsDeferredshader)
	{
		List->SetTexture(SceneRenderer::Get()->GetScene()->GetLightingData()->SkyBox, DeferredLightingShaderRSBinds::SpecBlurMap);
		List->SetFrameBufferTexture(GpuData[List->GetDeviceIndex()].SkyBoxBuffer, DeferredLightingShaderRSBinds::DiffuseIr);
		List->SetFrameBufferTexture(GpuData[List->GetDeviceIndex()].EnvMap->EnvBRDFBuffer, DeferredLightingShaderRSBinds::EnvBRDF);
	}
	else
	{
		//if (List->GetDeviceIndex() == 0)
		{
			List->SetTexture(SceneRenderer::Get()->GetScene()->GetLightingData()->SkyBox, "SpecularBlurMap");
			List->SetFrameBufferTexture(GpuData[List->GetDeviceIndex()].SkyBoxBuffer, "DiffuseIrMap");
			List->SetFrameBufferTexture(GpuData[List->GetDeviceIndex()].EnvMap->EnvBRDFBuffer, "envBRDFTexture");
		}
	}
}

void ReflectionEnviroment::GenerateStaticEnvData()
{
	for (int i = 0; i < RHI::GetDeviceCount(); i++)
	{
		if (RHI::GetFrameCount() == 0)
		{
			GpuData[i].EnvMap->ComputeEnvBRDF();
		}
		GpuData[i].Conv->ComputeConvolution(SceneRenderer::Get()->GetScene()->GetLightingData()->SkyBox, GpuData[i].SkyBoxBuffer);
	}

}

void ReflectionEnviroment::ReflectionEnviromentGPUData::init(int index)
{
	Conv = ShaderComplier::GetShader<Shader_Convolution>(RHI::GetDeviceContext(index));
	EnvMap = ShaderComplier::GetShader<Shader_EnvMap>(RHI::GetDeviceContext(index));
	Conv->init();
	EnvMap->Init();

	const int Size = 1024;
	RHIFrameBufferDesc Desc = RHIFrameBufferDesc::CreateCubeColourDepth(Size, Size);
	Desc.RTFormats[0] = eTEXTURE_FORMAT::FORMAT_R32G32B32A32_FLOAT;
	SkyBoxBuffer = RHI::CreateFrameBuffer(RHI::GetDeviceContext(index), Desc);
}
