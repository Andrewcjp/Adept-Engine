#include "SceneRenderer.h"
#include "Core/Assets/Scene.h"
#include "Core/BaseWindow.h"
#include "Core/GameObject.h"
#include "Core/Performance/PerfManager.h"
#include "FrameBufferProcessor.h"
#include "Material.h"
#include "Mesh/MeshPipelineController.h"
#include "ReflectionProbe.h"
#include "Rendering/Shaders/Shader_Main.h"
#include "Rendering/Shaders/Shader_NodeGraph.h"
#include "Rendering/Shaders/Shader_Skybox.h"
#include "Rendering/VR/VRCamera.h"
#include "LightCulling/LightCullingEngine.h"
#include "Culling/CullingManager.h"
#include "ParticleSystemManager.h"
#include "Editor/Editor_Camera.h"
#include "ReflectionEnviroment.h"
#include "../Shaders/Shadow/Shader_Depth.h"
#include "../Shaders/Raytracing/Shader_Skybox_Miss.h"
#include "../RayTracing/RayTracingEngine.h"
#include "Screen.h"
#include "ShadowRenderer.h"
#include "../Renderers/Terrain/TerrainRenderer.h"
#include "RHI/RHIBufferGroup.h"
#include "../RayTracing/Voxel/VoxelScene.h"

SceneRenderer* SceneRenderer::Instance = nullptr;
void SceneRenderer::StartUp()
{
	Instance = new SceneRenderer();
	new ShadowRenderer();
}

void SceneRenderer::Shutdown()
{
	SafeDelete(Instance);
}

SceneRenderer * SceneRenderer::Get()
{
	return Instance;
}

SceneRenderer::SceneRenderer()
{
	TargetScene = nullptr;
	WorldDefaultMatShader = (Shader_NodeGraph*)Material::GetDefaultMaterialShader();
	MeshController = new MeshPipelineController();
	Culling = new CullingManager();
	LightCulling = new LightCullingEngine();
	LightCulling->Init(Culling);
	Enviroment = new ReflectionEnviroment();
	Init();
	LightsBuffer = LightBufferW();
	float g_quad_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,0.0f,
		1.0f, -1.0f, 0.0f,0.0f,
		-1.0f,  1.0f, 0.0f,0.0f,
		-1.0f,  1.0f, 0.0f,0.0f,
		1.0f, -1.0f, 0.0f,0.0f,
		1.0f,  1.0f, 0.0f,0.0f,
	};
	QuadBuffer = new RHIBufferGroup();
	QuadBuffer->CreateVertexBuffer(sizeof(float) * 4, sizeof(float) * 6 * 4, EBufferAccessType::Dynamic);
	QuadBuffer->UpdateVertexBuffer(&g_quad_vertex_buffer_data, sizeof(float) * 6 * 4);
	TerrainRenderer::Get();
	if (RHI::GetRenderSettings()->GetVoxelSet().Enabled)
	{
		mVoxelScene = new VoxelScene();
	}
}


SceneRenderer::~SceneRenderer()
{
	MemoryUtils::RHIUtil::DeleteRHICArray(CLightBuffer, MAX_GPU_DEVICE_COUNT);
	MemoryUtils::RHIUtil::DeleteRHICArray(CMVBuffer, MAX_GPU_DEVICE_COUNT);
	MemoryUtils::DeleteVector(probes);
	SafeDelete(MeshController);
}

void SceneRenderer::PrepareSceneForRender()
{
	if (SceneChanged)
	{
		Enviroment->GenerateStaticEnvData();
	}
#if WITH_EDITOR
	if (EditorCam != nullptr && EditorCam->GetEnabled())
	{
		if (CurrentCamera != EditorCam->GetCamera())
		{
			CurrentCamera = EditorCam->GetCamera();
		}
	}
	else
#endif
	{
		CurrentCamera = TargetScene->GetCurrentRenderCamera();
	}

	UpdateMVForMainPass();
	Culling->UpdateMainPassFrustumCulling(CurrentCamera, TargetScene);


	PrepareData();
	LightCulling->RunLightBroadphase();
	ShadowRenderer::Get()->UpdateShadowAsignments();
	LightsBuffer.LightCount = LightCulling->GetNumLights();
	LightsBuffer.TileX = LightCulling->GetLightGridDim().x;
	LightsBuffer.TileY = LightCulling->GetLightGridDim().y;
	LightCulling->UpdateLightsBuffer();
	UpdateLightBuffer(TargetScene->GetLights());
	//#todo:move this to node
	ParticleSystemManager::Get()->PreRenderUpdate(CurrentCamera);
	if (mVoxelScene != nullptr)
	{
		mVoxelScene->Update();
	}
	SceneChanged = false;
}

void SceneRenderer::PrepareData()
{
	if (TargetScene == nullptr)
	{
		return;
	}
	MeshController->GatherBatches();
	for (size_t i = 0; i < TargetScene->GetMeshObjects().size(); i++)
	{
		TargetScene->GetMeshObjects()[i]->PrepareDataForRender();
	}
}

void SceneRenderer::RenderScene(RHICommandList * CommandList, bool PositionOnly, FrameBuffer* FrameBuffer, bool IsCubemap, int index /*=0*/)
{
	if (!PositionOnly)
	{
		BindLightsBufferB(CommandList, MainShaderRSBinds::LightDataCBV);
	}
	if (!IsCubemap)
	{
		BindMvBuffer(CommandList, "", index);
	}
	if (PositionOnly)
	{
		MeshController->RenderPass(ERenderPass::PreZ, CommandList);
	}
	else
	{
		MeshController->RenderPass(IsCubemap ? ERenderPass::BasePass_Cubemap : ERenderPass::BasePass, CommandList);
	}
}

void SceneRenderer::Init()
{
	for (int i = 0; i < RHI::GetDeviceCount(); i++)
	{
		CLightBuffer[i] = RHI::CreateRHIBuffer(ERHIBufferType::Constant, RHI::GetDeviceContext(i));
		CLightBuffer[i]->SetDebugName("Light buffer");
		CLightBuffer[i]->CreateConstantBuffer(sizeof(LightBufferW), 1);
		CMVBuffer[i] = RHI::CreateRHIBuffer(ERHIBufferType::Constant, RHI::GetDeviceContext(i));
		CMVBuffer[i]->SetDebugName("CMVBuffer");
		CMVBuffer[i]->CreateConstantBuffer(sizeof(MVBuffer), RHI::SupportVR() ? 2 : 1);
	}
	LastMVData = new RHIBufferGroup();
	GetLastMVData()->CreateConstantBuffer(sizeof(MVBuffer), 1);
}

void SceneRenderer::UpdateMV(VRCamera* c)
{
	UpdateMV(c->GetEyeCam(EEye::Left), EEye::Left);
	UpdateMV(c->GetEyeCam(EEye::Right), EEye::Right);
}

void SceneRenderer::UpdateMV(Camera * c, int index /*= 0*/)
{
	LastMVData->UpdateConstantBuffer(&MV_Buffer);

	MV_Buffer.V = c->GetView();
	MV_Buffer.P = c->GetProjection();
	MV_Buffer.CameraPos = c->GetPosition();
	MV_Buffer.INV_P = glm::inverse(MV_Buffer.P);
	MV_Buffer.Res = Screen::GetScaledRes();
	MV_Buffer.INV_res.x = 1.0f / (float)MV_Buffer.Res.x;
	MV_Buffer.INV_res.y = 1.0f / (float)MV_Buffer.Res.y;
	for (int i = 0; i < RHI::GetDeviceCount(); i++)
	{
		CMVBuffer[i]->UpdateConstantBuffer(&MV_Buffer, index);
	}
}

void SceneRenderer::UpdateMV(glm::mat4 View, glm::mat4 Projection)
{
	MV_Buffer.V = View;
	MV_Buffer.P = Projection;
	for (int i = 0; i < RHI::GetDeviceCount(); i++)
	{
		CMVBuffer[i]->UpdateConstantBuffer(&MV_Buffer, 0);
	}
}

void SceneRenderer::UpdateLightBuffer(std::vector<Light*> lights)
{
	for (int devindex = 0; devindex < RHI::GetDeviceCount(); devindex++)
	{
		int PreSampleIndex = 0;
		for (int i = 0; i < lights.size(); i++)
		{
			lights[i]->Update();
			if (i >= MAX_POSSIBLE_LIGHTS || i >= RHI::GetRenderConstants()->MAX_LIGHTS)
			{
				continue;
			}
			LightUniformBuffer newitem = CreateLightEntity(lights[i], devindex);

			//assume if not resident its pre-sampled
//			newitem.PreSampled[0] = !lights[i]->GPUShadowResidentMask[devindex];
			newitem.PreSampled[1] = PreSampleIndex;

			if (newitem.PreSampled[0])
			{
				PreSampleIndex++;
			}

			if (lights[i]->GetType() == ELightType::Directional || lights[i]->GetType() == ELightType::Spot)
			{
				glm::mat4 LightView = glm::lookAtLH<float>(lights[i]->GetPosition(), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));//world up
				glm::vec3 position = glm::vec3(0, 20, 50);
				position = lights[i]->GetPosition();
				LightView = glm::lookAtLH<float>(position, position + newitem.Direction, glm::vec3(0, 0, 1));//world up
				//float size = 100.0f;
//				glm::mat4 proj;
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
		////LightsBuffer.LightCount = 1;// lights.size();
		//const int TileSize = RHI::GetRenderConstants()->LIGHTCULLING_TILE_SIZE;
		//LightsBuffer.Tiles[0] = GetLightGridDim();
		//LightsBuffer.Tiles[1] = BaseWindow::GetCurrentRenderer()->GetScaledHeight() / TileSize;
		LightsBuffer.LightCount = LightCulling->GetNumLights();
		CLightBuffer[devindex]->UpdateConstantBuffer(&LightsBuffer, 0);
	}

}

LightUniformBuffer SceneRenderer::CreateLightEntity(Light * L, int devindex)
{
	L->Update();
	LightUniformBuffer newitem = {};
	newitem.LPosition = L->GetPosition();
	newitem.color = glm::vec3(L->GetColor());
	newitem.Direction = L->GetDirection();
	newitem.type = L->GetType();
	newitem.HasShadow = L->GetDoesShadow();
	ShadowRenderer::UpdateShadowID(L, devindex);
	newitem.ShadowID = L->GetShadowId();
	newitem.Range = L->GetRange();
	//newitem.PreSampled.x = true;
	return newitem;
}

void SceneRenderer::BindLightsBufferB(RHICommandList*  list, int Override)
{
	if (Override == -1)
	{
		return;
	}
	list->SetConstantBufferView(CLightBuffer[list->GetDeviceIndex()], 0, Override);
}

void SceneRenderer::BindLightsBuffer(RHICommandList*  list, std::string slot)
{
	if (slot.length() == 0)
	{
		slot = "LightBuffer";
	}
	list->SetConstantBufferView(CLightBuffer[list->GetDeviceIndex()], 0, slot);
}

void SceneRenderer::BindMvBuffer(RHICommandList * list, std::string slot, int index)
{
	if (slot.length() == 0)
	{
		slot = "SceneConstantBuffer";
	}
	list->SetConstantBufferView(CMVBuffer[list->GetDeviceIndex()], index, slot);
}

void SceneRenderer::BindMvBufferB(RHICommandList * list, int slot, int index)
{
	list->SetConstantBufferView(CMVBuffer[list->GetDeviceIndex()], index, slot);
}

void SceneRenderer::SetScene(Scene * NewScene)
{
	TargetScene = NewScene;
	MeshController->TargetScene = TargetScene;
	if (NewScene != nullptr)
	{
		ShaderComplier::GetShader<Shader_Skybox>()->SetSkyBox(NewScene->GetLightingData()->SkyBox);
	}
	//run update on scene data
	SceneChanged = true;
	if (RHI::GetRenderSettings()->RaytracingEnabled())
	{
		if (NewScene != nullptr)
		{
			ShaderComplier::GetShader<Shader_Skybox_Miss>()->SetSkybox(TargetScene->GetLightingData()->SkyBox);
		}
		RayTracingEngine::Get()->UpdateFromScene(TargetScene);
	}
	if (mVoxelScene != nullptr)
	{
		mVoxelScene->Create(NewScene);
	}
}


Scene * SceneRenderer::GetScene()
{
	return TargetScene;
}

LightCullingEngine * SceneRenderer::GetLightCullingEngine()
{
	return LightCulling;
}

MeshPipelineController * SceneRenderer::GetPipelineController()
{
	return MeshController;
}

CullingManager * SceneRenderer::GetCullingManager()
{
	return Culling;
}

Camera * SceneRenderer::GetCurrentCamera()
{
	return Get()->CurrentCamera;
}

void SceneRenderer::SetEditorCamera(Editor_Camera * Cam)
{
	EditorCam = Cam;
}

ReflectionEnviroment * SceneRenderer::GetReflectionEnviroment()
{
	return Enviroment;
}

void SceneRenderer::DrawScreenQuad(RHICommandList * list)
{
	list->SetVertexBuffer(SceneRenderer::Get()->QuadBuffer->Get(list));  
	list->DrawPrimitive(6, 1, 0, 0);
}


void SceneRenderer::SetupBindsForForwardPass(RHICommandList * list, int eyeindex, FrameBuffer* TargetBuffer)
{
	//push a pso for bindings
	RHIPipeLineStateDesc Desc = RHIPipeLineStateDesc::CreateDefault(Material::GetDefaultMaterial()->GetShaderInstance(EMaterialPassType::Forward));
	Desc.RenderTargetDesc = TargetBuffer->GetPiplineRenderDesc();
	list->SetPipelineStateDesc(Desc);
	BindMvBuffer(list, "", eyeindex);
	BindLightsBuffer(list, "");

}

void SceneRenderer::UpdateMVForMainPass()
{
	if (RHI::IsRenderingVR())
	{
		VRCamera* VRCam = RHI::GetHMD()->GetVRCamera();
		UpdateMV(VRCam);
	}
	else
	{
		float Aspect = (float)Screen::GetScaledWidth() / (float)Screen::GetScaledHeight();
		CurrentCamera->UpdateProjection(Aspect);
		UpdateMV(CurrentCamera);
	}
}