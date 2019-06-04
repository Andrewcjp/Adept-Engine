
#include "SceneRenderer.h"
#include "Rendering/Shaders/Shader_Main.h"
#include "Core/GameObject.h"
#include "Core/Assets/Scene.h"
#include "Rendering/Shaders/Shader_NodeGraph.h"
#include "Rendering/Core/RelfectionProbe.h"
#include "Rendering/Core/Material.h"
#include "Core/Performance/PerfManager.h"
#include "Mesh/MeshPipelineController.h"
#include "FrameBufferProcessor.h"
#include "../VR/VRCamera.h"
#include "../Shaders/Shader_Skybox.h"
#include "Core/BaseWindow.h"
#include "../Renderers/RenderEngine.h"
SceneRenderer::SceneRenderer(Scene* Target)
{
	TargetScene = Target;
	WorldDefaultMatShader = (Shader_NodeGraph*)Material::GetDefaultMaterialShader();
	Controller = new MeshPipelineController();
}


SceneRenderer::~SceneRenderer()
{
	MemoryUtils::RHIUtil::DeleteRHICArray(CLightBuffer, MAX_GPU_DEVICE_COUNT);
	MemoryUtils::DeleteVector(probes);
	EnqueueSafeRHIRelease(CMVBuffer);
	SafeDelete(Controller);
	EnqueueSafeRHIRelease(RelfectionProbeProjections);
}

void SceneRenderer::RenderScene(RHICommandList * CommandList, bool PositionOnly, FrameBuffer* FrameBuffer, bool IsCubemap, int index /*=0*/)
{
	if (!PositionOnly)
	{
		BindLightsBuffer(CommandList, MainShaderRSBinds::LightDataCBV);
	}
	if (!IsCubemap)
	{
		BindMvBuffer(CommandList, MainShaderRSBinds::MVCBV, index);
	}
	if (PositionOnly)
	{
		Controller->RenderPass(ERenderPass::PreZ, CommandList);
	}
	else
	{
		Controller->RenderPass(IsCubemap ? ERenderPass::BasePass_Cubemap : ERenderPass::BasePass, CommandList);
	}
}

void SceneRenderer::Init()
{
	for (int i = 0; i < RHI::GetDeviceCount(); i++)
	{
		CLightBuffer[i] = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
		CLightBuffer[i]->SetDebugName("Light buffer");
		CLightBuffer[i]->CreateConstantBuffer(sizeof(LightBufferW), 1, true);
	}
	CMVBuffer = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
	CMVBuffer->SetDebugName("CMVBuffer");
	CMVBuffer->CreateConstantBuffer(sizeof(MVBuffer), RHI::SupportVR() ? 2 : 1, true);
	RelfectionProbeProjections = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
	RelfectionProbeProjections->CreateConstantBuffer(sizeof(MVBuffer), 6, true);

	UpdateReflectionParams(glm::vec3(0, 5, 0));
}
void SceneRenderer::UpdateReflectionParams(glm::vec3 lightPos)
{
	return;
	glm::mat4x4 shadowProj = glm::perspectiveLH<float>(glm::radians(90.0f), 1.0f, zNear, ZFar);
	glm::mat4x4 transforms[6];
	transforms[0] = (glm::lookAtRH(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0)));
	transforms[1] = (glm::lookAtRH(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0)));
	transforms[2] = (glm::lookAtRH(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
	transforms[3] = (glm::lookAtRH(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
	transforms[4] = (glm::lookAtRH(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, 1.0, 0.0)));
	transforms[5] = (glm::lookAtRH(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 1.0, 0.0)));
	for (int i = 0; i < 6; i++)
	{
		CubeMapViews[i].P = shadowProj;
		CubeMapViews[i].V = transforms[i];
		CubeMapViews[i].CameraPos = lightPos;
	}
	for (int i = 0; i < 6; i++)
	{
		RelfectionProbeProjections->UpdateConstantBuffer(&CubeMapViews[i], i);
	}
}

void SceneRenderer::UpdateMV(VRCamera* c)
{
	UpdateMV(c->GetEyeCam(EEye::Left), EEye::Left);
	UpdateMV(c->GetEyeCam(EEye::Right), EEye::Right);
}

void SceneRenderer::UpdateMV(Camera * c, int index /*= 0*/)
{
	MV_Buffer.V = c->GetView();
	MV_Buffer.P = c->GetProjection();
	MV_Buffer.CameraPos = c->GetPosition();
	CMVBuffer->UpdateConstantBuffer(&MV_Buffer, index);
}

void SceneRenderer::UpdateMV(glm::mat4 View, glm::mat4 Projection)
{
	MV_Buffer.V = View;
	MV_Buffer.P = Projection;
	CMVBuffer->UpdateConstantBuffer(&MV_Buffer, 0);
}

void SceneRenderer::UpdateLightBuffer(std::vector<Light*> lights)
{
	for (int devindex = 0; devindex < RHI::GetDeviceCount(); devindex++)
	{
		int PreSampleIndex = 0;
		for (int i = 0; i < lights.size(); i++)
		{
			if (i >= MAX_POSSIBLE_LIGHTS || i >= RHI::GetRenderConstants()->MAX_LIGHTS)
			{
				continue;
			}
			lights[i]->Update();
			LightUniformBuffer newitem = {};
			newitem.position = lights[i]->GetPosition();
			newitem.color = glm::vec3(lights[i]->GetColor());
			newitem.Direction = lights[i]->GetDirection();
			newitem.type = lights[i]->GetType();
			newitem.HasShadow = lights[i]->GetDoesShadow();
			//assume if not resident its pre-sampled
			newitem.PreSampled[0] = !lights[i]->GPUShadowResidentMask[devindex];
			newitem.PreSampled[1] = PreSampleIndex;
			newitem.Range = lights[i]->GetRange();
			if (newitem.PreSampled[0])
			{
				PreSampleIndex++;
			}
			newitem.ShadowID = lights[i]->GetShadowId();
			if (lights[i]->GetType() == ELightType::Directional || lights[i]->GetType() == ELightType::Spot)
			{
				glm::mat4 LightView = glm::lookAtLH<float>(lights[i]->GetPosition(), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));//world up
				glm::vec3 position = glm::vec3(0, 20, 50);
				position = lights[i]->GetPosition();
				LightView = glm::lookAtLH<float>(position, position + newitem.Direction, glm::vec3(0, 0, 1));//world up
				float size = 100.0f;
				glm::mat4 proj;
				if (lights[i]->GetType() == ELightType::Spot)
				{
					//proj = glm::perspective<float>(glm::radians(45.0f), 1.0f, 2.0f, 50.0f);
					LightView = glm::lookAtLH<float>(lights[i]->GetPosition(), lights[i]->GetPosition() + newitem.Direction, glm::vec3(0, 0, 1));//world up
				}
				else
				{
					//proj = glm::orthoLH<float>(-size, size, -size, size, -200, 100);
				}
				//lights[i]->Projection = proj;
				lights[i]->DirView = LightView;
				//				newitem.LightVP = proj * LightView;
			}
			if (lights[i]->GetType() == ELightType::Point)
			{
				float znear = 1.0f;
				float zfar = 500;
				glm::mat4 proj = glm::perspectiveLH<float>(glm::radians(90.0f), 1.0f, znear, zfar);
				lights[i]->Projection = proj;
			}
			LightsBuffer.Light[i] = newitem;
		}
		LightsBuffer.LightCount = lights.size();
		const int TileSize = RHI::GetRenderConstants()->LIGHTCULLING_TILE_SIZE;
		LightsBuffer.Tiles[0] = BaseWindow::GetCurrentRenderer()->GetScaledWidth() / TileSize;
		LightsBuffer.Tiles[1] = BaseWindow::GetCurrentRenderer()->GetScaledHeight() / TileSize;
		CLightBuffer[devindex]->UpdateConstantBuffer(&LightsBuffer, 0);
	}
}

void SceneRenderer::BindLightsBuffer(RHICommandList*  list, int Override)
{
	list->SetConstantBufferView(CLightBuffer[list->GetDeviceIndex()], 0, Override);
}

void SceneRenderer::BindMvBuffer(RHICommandList * list, int slot)
{
	BindMvBuffer(list, slot, 0);
}

void SceneRenderer::BindMvBuffer(RHICommandList * list, int slot, int index)
{
	list->SetConstantBufferView(CMVBuffer, index, slot);
}

void SceneRenderer::SetScene(Scene * NewScene)
{
	TargetScene = NewScene;
	Controller->TargetScene = TargetScene;
}

void SceneRenderer::UpdateRelflectionProbes(RHICommandList* commandlist)
{
	SCOPE_CYCLE_COUNTER_GROUP("Update Relflection Probes", "Render");
	commandlist->StartTimer(EGPUTIMERS::CubemapCapture);
	for (int i = 0; i < probes.size(); i++)
	{
		RelfectionProbe* Probe = probes[i];
		RenderCubemap(Probe, commandlist);
	}
	commandlist->EndTimer(EGPUTIMERS::CubemapCapture);
}

bool SceneRenderer::AnyProbesNeedUpdate()
{
	for (int i = 0; i < probes.size(); i++)
	{
		if (probes[i]->NeedsCapture())
		{
			return true;
		}
	}
	return false;
}

Scene * SceneRenderer::GetScene()
{
	return TargetScene;
}

void SceneRenderer::RenderCubemap(RelfectionProbe * Map, RHICommandList* commandlist)
{
	if (!Map->NeedsCapture())
	{
		return;
	}
	commandlist->ClearFrameBuffer(Map->CapturedTexture);
	RHIPipeLineStateDesc Desc = RHIPipeLineStateDesc::CreateDefault(Material::GetDefaultMaterialShader(), Map->CapturedTexture);
	for (int i = 0; i < 6; i++)
	{
		commandlist->SetPipelineStateDesc(Desc);
		SetMVForProbe(commandlist, i, MainShaderRSBinds::MVCBV);
		RHIRenderPassDesc Info;
		Info.TargetBuffer = Map->CapturedTexture;
		commandlist->BeginRenderPass(Info);
		//commandlist->SetRenderTarget(Map->CapturedTexture, i);
		RenderScene(commandlist, false, Map->CapturedTexture, true);
		SB->Render(this, commandlist, Map->CapturedTexture, nullptr, true, i);
		commandlist->EndRenderPass();
	}
	Map->SetCaptured();
	//FrameBufferProcessor::CreateMipChain(Map->CapturedTexture, commandlist);
}

void SceneRenderer::SetMVForProbe(RHICommandList* list, int index, int Slot)
{
	list->SetConstantBufferView(RelfectionProbeProjections, index, Slot);
}

void SceneRenderer::SetupBindsForForwardPass(RHICommandList * list, int eyeindex)
{
	RHIPipeLineStateDesc Desc = RHIPipeLineStateDesc::CreateDefault(Material::GetDefaultMaterial()->GetShaderInstance(EMaterialPassType::Forward));
	list->SetPipelineStateDesc(Desc);
	BindMvBuffer(list, MainShaderRSBinds::MVCBV, eyeindex);
	BindLightsBuffer(list, MainShaderRSBinds::LightDataCBV);

}