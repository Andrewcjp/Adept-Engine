
#include "SceneRenderer.h"
#include "Rendering/Shaders/Shader_Main.h"
#include "Core/GameObject.h"
#include "Core/Assets/Scene.h"
#include "Rendering/Shaders/Shader_NodeGraph.h"
#include "Rendering/Core/RelfectionProbe.h"
#include "Rendering/Core/Material.h"
#include "Core/Performance/PerfManager.h"
SceneRenderer::SceneRenderer(Scene* Target)
{
	TargetScene = Target;
	WorldDefaultMatShader = (Shader_NodeGraph*)Material::GetDefaultMaterialShader();
}


SceneRenderer::~SceneRenderer()
{
	MemoryUtils::RHIUtil::DeleteRHICArray(CLightBuffer, MAX_GPU_DEVICE_COUNT);
	EnqueueSafeRHIRelease(CMVBuffer);
	EnqueueSafeRHIRelease(GameObjectTransformBuffer);
	EnqueueSafeRHIRelease(RelfectionProbeProjections);
}

void SceneRenderer::RenderScene(RHICommandList * CommandList, bool PositionOnly, FrameBuffer* FrameBuffer, bool IsCubemap)
{
	if (!PositionOnly)
	{
		BindLightsBuffer(CommandList, MainShaderRSBinds::LightDataCBV);
	}
	if (!IsCubemap)
	{
		BindMvBuffer(CommandList);
	}
	for (int i = 0; i < (*TargetScene->GetMeshObjects()).size(); i++)
	{
		GameObject* CurrentObj = (*TargetScene->GetMeshObjects())[i];
		SetActiveIndex(CommandList, (int)i, CommandList->GetDeviceIndex());
		CurrentObj->Render(PositionOnly, CommandList);
	}
}

void SceneRenderer::Init()
{
	for (int i = 0; i < MaxConstant; i++)
	{
		SceneBuffer.push_back(SceneConstantBuffer());
	}
	UpdateTransformBufferSize(MaxConstant);

	for (int i = 0; i < RHI::GetDeviceCount(); i++)
	{
		CLightBuffer[i] = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
		CLightBuffer[i]->CreateConstantBuffer(sizeof(LightBufferW), 1, true);
	}
	CMVBuffer = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
	CMVBuffer->CreateConstantBuffer(sizeof(MVBuffer), 1, true);
	RelfectionProbeProjections = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
	RelfectionProbeProjections->CreateConstantBuffer(sizeof(MVBuffer), 6, true);

	UpdateReflectionParams(glm::vec3(0, 5, 0));
}
void SceneRenderer::UpdateReflectionParams(glm::vec3 lightPos)
{
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

void SceneRenderer::UpdateCBV()
{
	for (int i = 0; i < MaxConstant; i++)
	{
		GameObjectTransformBuffer->UpdateConstantBuffer(&SceneBuffer[i], i);
	}
}

void SceneRenderer::UpdateUnformBufferEntry(const SceneConstantBuffer &bufer, int index)
{
	if (index < MaxConstant)
	{
		SceneBuffer[index] = bufer;
	}
	else
	{
		UpdateTransformBufferSize(index + 10);
	}
}

void SceneRenderer::UpdateTransformBufferSize(int NewSize)
{
	EnqueueSafeRHIRelease(GameObjectTransformBuffer);
	GameObjectTransformBuffer = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
	GameObjectTransformBuffer->CreateConstantBuffer(sizeof(SceneConstantBuffer), NewSize, true);
	SceneBuffer.resize(NewSize);
	MaxConstant = NewSize;
}

void SceneRenderer::SetActiveIndex(RHICommandList* list, int index, int DeviceIndex)
{
	list->SetConstantBufferView(GameObjectTransformBuffer, index, MainShaderRSBinds::GODataCBV);
}

void SceneRenderer::UpdateMV(Camera * c)
{
	MV_Buffer.V = c->GetView();
	MV_Buffer.P = c->GetProjection();
	MV_Buffer.CameraPos = c->GetPosition();
	CMVBuffer->UpdateConstantBuffer(&MV_Buffer, 0);
}

void SceneRenderer::UpdateMV(glm::mat4 View, glm::mat4 Projection)
{
	MV_Buffer.V = View;
	MV_Buffer.P = Projection;
	CMVBuffer->UpdateConstantBuffer(&MV_Buffer, 0);
}

SceneConstantBuffer SceneRenderer::CreateUnformBufferEntry(GameObject * t)
{
	SceneConstantBuffer m_constantBufferData;
	m_constantBufferData.M = t->GetTransform()->GetModel();
	m_constantBufferData.HasNormalMap = false;
	if (t->GetMesh()->GetMaterial(0) != nullptr)
	{
		m_constantBufferData.HasNormalMap = t->GetMesh()->GetMaterial(0)->HasNormalMap();
		m_constantBufferData.Metallic = t->GetMesh()->GetMaterial(0)->GetProperties()->Metallic;
		m_constantBufferData.Roughness = t->GetMesh()->GetMaterial(0)->GetProperties()->Roughness;
	}
	//used in the prepare stage for this frame!
	return m_constantBufferData;
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
			LightUniformBuffer newitem;
			newitem.position = lights[i]->GetPosition();
			newitem.color = glm::vec3(lights[i]->GetColor());
			newitem.Direction = lights[i]->GetDirection();
			newitem.type = lights[i]->GetType();
			newitem.HasShadow = lights[i]->GetDoesShadow();
			//assume if not resident its pre-sampled
			newitem.PreSampled[0] = !lights[i]->GPUShadowResidentMask[devindex];
			newitem.PreSampled[1] = PreSampleIndex;
			if (newitem.PreSampled[0])
			{
				PreSampleIndex++;
			}
			newitem.ShadowID = lights[i]->GetShadowId();
			if (lights[i]->GetType() == Light::Directional || lights[i]->GetType() == Light::Spot)
			{
				glm::mat4 LightView = glm::lookAtLH<float>(lights[i]->GetPosition(), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));//world up
				glm::vec3 position = glm::vec3(0, 20, 50);
				position = lights[i]->GetPosition();
				LightView = glm::lookAtLH<float>(position, position + newitem.Direction, glm::vec3(0, 0, 1));//world up
				float size = 100.0f;
				glm::mat4 proj;
				if (lights[i]->GetType() == Light::Spot)
				{
					proj = glm::perspective<float>(glm::radians(45.0f), 1.0f, 2.0f, 50.0f);
					LightView = glm::lookAtLH<float>(lights[i]->GetPosition(), lights[i]->GetPosition() + newitem.Direction, glm::vec3(0, 0, 1));//world up
				}
				else
				{
					proj = glm::orthoLH<float>(-size, size, -size, size, -200, 100);
				}
				lights[i]->Projection = proj;
				lights[i]->DirView = LightView;
				newitem.LightVP = proj * LightView;
			}
			if (lights[i]->GetType() == Light::Point)
			{
				float znear = 1.0f;
				float zfar = 500;
				glm::mat4 proj = glm::perspectiveLH<float>(glm::radians(90.0f), 1.0f, znear, zfar);
				lights[i]->Projection = proj;
			}
			LightsBuffer.Light[i] = newitem;
		}
		CLightBuffer[devindex]->UpdateConstantBuffer(&LightsBuffer, 0);
	}
}

void SceneRenderer::BindLightsBuffer(RHICommandList*  list, int Override)
{
	list->SetConstantBufferView(CLightBuffer[list->GetDeviceIndex()], 0, Override);
}

void SceneRenderer::BindMvBuffer(RHICommandList * list)
{
	list->SetConstantBufferView(CMVBuffer, 0, MainShaderRSBinds::MVCBV);
}

void SceneRenderer::BindMvBuffer(RHICommandList * list, int slot)
{
	list->SetConstantBufferView(CMVBuffer, 0, slot);
}

void SceneRenderer::SetScene(Scene * NewScene)
{
	TargetScene = NewScene;
}
void SceneRenderer::ClearBuffer()
{
	SceneBuffer.empty();
}
void SceneRenderer::UpdateRelflectionProbes(std::vector<RelfectionProbe*> & probes, RHICommandList* commandlist)
{
	SCOPE_CYCLE_COUNTER_GROUP("Update Relflection Probes","Render");
	commandlist->StartTimer(EGPUTIMERS::CubemapCapture);
	for (int i = 0; i < probes.size(); i++)
	{
		RelfectionProbe* Probe = probes[i];
		RenderCubemap(Probe, commandlist);
	}
	commandlist->EndTimer(EGPUTIMERS::CubemapCapture);
}
void SceneRenderer::RenderCubemap(RelfectionProbe * Map, RHICommandList* commandlist)
{

	commandlist->ClearFrameBuffer(Map->CapturedTexture);
	for (int i = 0; i < 6; i++)
	{
		commandlist->SetConstantBufferView(RelfectionProbeProjections, i, MainShaderRSBinds::MVCBV);
		commandlist->SetRenderTarget(Map->CapturedTexture, i);
		RenderScene(commandlist, false, Map->CapturedTexture, true);
	}
}
