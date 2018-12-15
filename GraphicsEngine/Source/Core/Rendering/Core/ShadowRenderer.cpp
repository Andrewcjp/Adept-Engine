#include "ShadowRenderer.h"
#include "Rendering/Shaders/Shader_Main.h"
#include "Rendering\Shaders\Shader_Depth.h"
#include "Rendering/Shaders/MultiGPU/Shader_ShadowSample.h"
#include "Core/GameObject.h"
#include "Core/Performance/PerfManager.h"
#include "RHI/DeviceContext.h"
#include "Rendering/Core/SceneRenderer.h"
#include "RHI/RHITypes.h"

#define CUBE_SIDES 6
#define TEST_PRESAMPLE 1
ShadowRenderer::ShadowRenderer(SceneRenderer * sceneRenderer)
{
	Scenerenderer = sceneRenderer;
	DirectionalLightShader = ShaderComplier::GetShader_Default<Shader_Depth>(false);

	int shadowwidth = 1024;
	ShadowDirectionalArray = RHI::CreateTextureArray(RHI::GetDeviceContext(0), RHI::GetRenderConstants()->MAX_DYNAMIC_DIRECTIONAL_SHADOWS);
	for (int i = 0; i < RHI::GetRenderConstants()->MAX_DYNAMIC_DIRECTIONAL_SHADOWS; i++)
	{
		DirectionalLightBuffers.push_back(RHI::CreateFrameBuffer(RHI::GetDeviceContext(0), RHIFrameBufferDesc::CreateDepth(shadowwidth, shadowwidth)));
		ShadowDirectionalArray->AddFrameBufferBind(DirectionalLightBuffers[i], i);
	}
	ShadowCubeArray = RHI::CreateTextureArray(RHI::GetDeviceContext(0), RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS);
#if !TEST_PRESAMPLE
	for (int i = 0; i < MAX_POINT_SHADOWS; i++)
	{
		PointLightBuffers.push_back(RHI::CreateFrameBuffer(RHI::GetDeviceContext(0), RHIFrameBufferDesc::CreateCubeDepth(shadowwidth, shadowwidth)));
		ShadowCubeArray->AddFrameBufferBind(PointLightBuffers[i], i);
	}
#else

	const int ShadowMapSize = RHI::GetRenderSettings()->ShadowMapSize;
	for (int i = 0; i < RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS; i++)
	{
		if ((i == 2 /*|| i== 3*/) && RHI::GetMGPUMode()->SplitShadowWork)
		{
			LightInteractions.push_back(new ShadowLightInteraction(RHI::GetDeviceContext(1), true, ShadowMapSize));
			ShadowCubeArray->SetIndexNull(2);
		}
		else
		{
			LightInteractions.push_back(new ShadowLightInteraction(RHI::GetDeviceContext(0), true, ShadowMapSize));
			ShadowCubeArray->AddFrameBufferBind(LightInteractions[i]->ShadowMap, i);
		}
	}
#endif
	DeviceContext* pointlightdevice = RHI::GetDeviceContext(0);
	if (RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS > 0)
	{
		PointLightShader = ShaderComplier::GetShader<Shader_Depth>(pointlightdevice,true); 
		GeometryProjections = RHI::CreateRHIBuffer(ERHIBufferType::Constant, pointlightdevice);
		GeometryProjections->CreateConstantBuffer(sizeof(glm::mat4) * CUBE_SIDES, RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS, true);
		PointShadowList = RHI::CreateCommandList(ECommandListType::Graphics, pointlightdevice);
		PointShadowListALT = RHI::CreateCommandList(ECommandListType::Graphics, RHI::GetDeviceContext(1));
		NAME_RHI_OBJECT(PointShadowListALT);
		NAME_RHI_OBJECT(PointShadowList);
	}
	DirectionalShadowList = RHI::CreateCommandList();
	NAME_RHI_OBJECT(DirectionalShadowList);
#if TEST_PRESAMPLE
	ShadowPreSampleShader = ShaderComplier::GetShader<Shader_ShadowSample>(pointlightdevice);
	ShadowPreSamplingList = RHI::CreateCommandList(ECommandListType::Graphics, RHI::GetDeviceContext(1));
	if (LightInteractions.size() > 2)
	{
		ShadowPreSamplingList->CreatePipelineState(ShadowPreSampleShader, LightInteractions[2]->PreSampledBuffer);
	}
#endif
}

ShadowRenderer::~ShadowRenderer()
{
	EnqueueSafeRHIRelease(GeometryProjections);
	EnqueueSafeRHIRelease(ShadowCubeArray);
	EnqueueSafeRHIRelease(ShadowDirectionalArray);
	EnqueueSafeRHIRelease(PointShadowListALT);
	EnqueueSafeRHIRelease(PointShadowList);
	EnqueueSafeRHIRelease(DirectionalShadowList);
	EnqueueSafeRHIRelease(ShadowPreSamplingList);
	MemoryUtils::RHIUtil::DeleteVector(DirectionalLightBuffers);
	MemoryUtils::DeleteVector(LightInteractions);
}

void ShadowRenderer::UpdateGeometryShaderParams(glm::vec3 lightPos, glm::mat4 shadowProj, int index)
{
	glm::mat4 transforms[6];
	transforms[0] = (shadowProj *
		glm::lookAtRH(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0)));
	transforms[1] = (shadowProj *
		glm::lookAtRH(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0)));
	transforms[2] = (shadowProj *
		glm::lookAtRH(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
	transforms[3] = (shadowProj *
		glm::lookAtRH(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
	transforms[4] = (shadowProj *
		glm::lookAtRH(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, 1.0, 0.0)));
	transforms[5] = (shadowProj *
		glm::lookAtRH(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 1.0, 0.0)));

	GeometryProjections->UpdateConstantBuffer(transforms, index);
}

void ShadowRenderer::RenderShadowMaps(Camera * c, std::vector<Light*>& lights, const std::vector<GameObject*>& ShadowObjects, Shader_Main* mainshader)
{
	SCOPE_CYCLE_COUNTER("Shadow CPU");
	if (UseCache)
	{
		if (Renderered)
		{
			return;
		}
		Renderered = true;
	}
	if (ShadowingDirectionalLights.size() > 0)
	{
		DirectionalShadowList->ResetList();
		DirectionalShadowList->GetDevice()->GetTimeManager()->StartTotalGPUTimer(DirectionalShadowList);
		DirectionalShadowList->GetDevice()->GetTimeManager()->StartTimer(DirectionalShadowList, EGPUTIMERS::DirShadows);
		RenderDirectionalShadows(DirectionalShadowList, mainshader, ShadowObjects);
		DirectionalShadowList->GetDevice()->GetTimeManager()->EndTimer(DirectionalShadowList, EGPUTIMERS::DirShadows);
		DirectionalShadowList->Execute();
	}
	if (ShadowingPointLights.size() > 0)
	{
		RunPointShadowPass(PointShadowList, ShadowObjects, mainshader);
		if (RHI::GetMGPUMode()->SplitShadowWork)
		{
			RunPointShadowPass(PointShadowListALT, ShadowObjects, mainshader);
			PreSampleShadows(ShadowObjects, mainshader);
		}
	}
}

void ShadowRenderer::RunPointShadowPass(RHICommandList* List, const std::vector<GameObject*>& ShadowObjects, Shader_Main* mainshader)
{
	List->ResetList();
	List->GetDevice()->GetTimeManager()->StartTotalGPUTimer(List);
	List->GetDevice()->GetTimeManager()->StartTimer(List, EGPUTIMERS::PointShadows);
	RenderPointShadows(List, mainshader, ShadowObjects);
	List->GetDevice()->GetTimeManager()->EndTimer(List, EGPUTIMERS::PointShadows);
	List->Execute();
}

void ShadowRenderer::PreSampleShadows(const std::vector<GameObject*>& ShadowObjects, Shader_Main* mainshader)
{
	SCOPE_CYCLE_COUNTER("PreSampleShadows");

	RHICommandList* list = ShadowPreSamplingList;
	list->ResetList();
	list->StartTimer(EGPUTIMERS::ShadowPreSample);
	for (int SNum = 0; SNum < (int)ShadowingPointLights.size(); SNum++)
	{
		if (!LightInteractions[SNum]->NeedsSample)
		{
			continue;
		}
		list->SetFrameBufferTexture(LightInteractions[SNum]->ShadowMap, Shader_ShadowSample::ShadowSRV);
		list->SetRenderTarget(LightInteractions[SNum]->PreSampledBuffer);
		list->ClearFrameBuffer(LightInteractions[SNum]->PreSampledBuffer);

		Scenerenderer->BindMvBuffer(list, Shader_Depth_RSSlots::VPBuffer);
		for (size_t i = 0; i < ShadowObjects.size(); i++)
		{
			if (ShadowObjects[i]->GetMesh() == nullptr)
			{
				//object should not be rendered to the depth map
				continue;
			}
			if (ShadowObjects[i]->GetMesh()->GetDoesShadow() == false)
			{
				continue;
			}
			Scenerenderer->SetActiveIndex(list, (int)i, list->GetDeviceIndex());
			ShadowObjects[i]->Render(true, list);
		}
		list->SetRenderTarget(nullptr);
	}
	list->EndTimer(EGPUTIMERS::ShadowPreSample);
	list->GetDevice()->GetTimeManager()->EndTotalGPUTimer(list);
	list->Execute();
	for (int SNum = 0; SNum < (int)ShadowingPointLights.size(); SNum++)
	{
		if (!LightInteractions[SNum]->NeedsSample)
		{
			continue;
		}
		FrameBuffer::CopyHelper(LightInteractions[SNum]->PreSampledBuffer, RHI::GetDeviceContext(0));
	}
}

void ShadowRenderer::RenderPointShadows(RHICommandList * list, Shader_Main * mainshader, const std::vector<GameObject *> & ShadowObjects)
{
	for (int SNum = 0; SNum < (int)ShadowingPointLights.size(); SNum++)
	{
		if (list->GetDeviceIndex() != LightInteractions[SNum]->DeviceIndex)
		{
			continue;
		}
		FrameBuffer* TargetBuffer = LightInteractions[SNum]->ShadowMap;
		Shader_Depth* TargetShader = LightInteractions[SNum]->Shader;
		list->SetRenderTarget(TargetBuffer);
		list->ClearFrameBuffer(TargetBuffer);
		//list->SetPipelineStateObject(TargetShader);
		UpdateGeometryShaderParams(ShadowingPointLights[SNum]->GetPosition(), ShadowingPointLights[SNum]->Projection, SNum);
		list->SetConstantBufferView(GeometryProjections, SNum, Shader_Depth_RSSlots::GeometryProjections);
		Shader_Depth::LightData data = {};
		data.Proj = ShadowingPointLights[SNum]->Projection;
		data.Lightpos = ShadowingPointLights[SNum]->GetPosition();
		TargetShader->UpdateBuffer(list, &data, SNum);
		for (size_t i = 0; i < ShadowObjects.size(); i++)
		{
			if (ShadowObjects[i]->GetMesh() == nullptr)
			{
				//object should not be rendered to the depth map
				continue;
			}
			if (ShadowObjects[i]->GetMesh()->GetDoesShadow() == false)
			{
				continue;
			}
			Scenerenderer->SetActiveIndex(list, (int)i, list->GetDeviceIndex());//default to Shader_Depth_RSSlots::ModelBuffer
			ShadowObjects[i]->Render(true, list);
		}
	}
	if (list->GetDeviceIndex() == 0)
	{
		list->InsertGPUStallTimer();
	}
}

void ShadowRenderer::RenderDirectionalShadows(RHICommandList * list, Shader_Main * mainshader, const std::vector<GameObject *> & ShadowObjects)
{
	for (size_t SNum = 0; SNum < ShadowingDirectionalLights.size(); SNum++)
	{
		FrameBuffer* TargetBuffer = DirectionalLightBuffers[SNum];

		list->SetRenderTarget(TargetBuffer);
		list->ClearFrameBuffer(TargetBuffer);
		//		mainshader->UpdateMV(ShadowingDirectionalLights[SNum]->DirView, ShadowingDirectionalLights[SNum]->Projection);
		Shader_Depth::LightData data = {};
		data.Proj = ShadowingDirectionalLights[SNum]->Projection;
		data.Lightpos = ShadowingDirectionalLights[SNum]->GetPosition();
		DirectionalLightShader->UpdateBuffer(list, &data, (int)SNum);
		for (size_t i = 0; i < ShadowObjects.size(); i++)
		{
			if (ShadowObjects[i]->GetMesh() == nullptr)
			{
				//object should not be rendered to the depth map
				continue;
			}
			if (ShadowObjects[i]->GetMesh()->GetDoesShadow() == false)
			{
				continue;
			}
			//mainshader->SetActiveIndex(list, (int)i);
			ShadowObjects[i]->Render(true, list);
		}
		//list->SetRenderTarget(nullptr);
	}
}

void ShadowRenderer::BindShadowMapsToTextures(RHICommandList * list)
{
#if 1
	if (RHI::GetRenderSettings()->IsDeferred)
	{
		ShadowDirectionalArray->BindToShader(list, 5);
		ShadowCubeArray->BindToShader(list, 6);
	}
	else
	{
		ShadowDirectionalArray->BindToShader(list, MainShaderRSBinds::DirShadow);
		ShadowCubeArray->BindToShader(list, MainShaderRSBinds::PointShadow);
	}
#else
	if (RHI::GetMGPUMode()->SplitShadowWork)
	{
#if 1
		list->SetFrameBufferTexture(LightInteractions[2]->PreSampledBuffer, 8);
#else
		list->SetFrameBufferTexture(LightInteractions[2]->PreSampledBuffer, 10);
#endif
	}
#endif
}

void ShadowRenderer::ClearShadowLights()
{
	ShadowingDirectionalLights.clear();
	ShadowingPointLights.clear();
	Renderered = false;
}

void ShadowRenderer::InitShadows(std::vector<Light*> lights)
{
	ClearShadowLights();
	if (lights.size() == 0)
	{
		return;
	}
	for (size_t i = 0; i < lights.size(); i++)
	{
		if (lights[i]->GetDoesShadow() == false)
		{
			continue;
		}
		if (lights[i]->GetType() == Light::Point)
		{
			if (ShadowingPointLights.size() < RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS)
			{
				lights[i]->SetShadowId((int)ShadowingPointLights.size());
				ShadowingPointLights.push_back(lights[i]);
			}
			else
			{
				Log::OutS << "Max point Shadows Hit" << Log::OutS;
				lights[i]->SetShadow(false);
			}
		}
		else if (lights[i]->GetType() == Light::Directional)
		{
			if (ShadowingDirectionalLights.size() < RHI::GetRenderConstants()->MAX_DYNAMIC_DIRECTIONAL_SHADOWS)
			{
				lights[i]->SetShadowId((int)ShadowingDirectionalLights.size());
				ShadowingDirectionalLights.push_back(lights[i]);
			}
			else
			{
				Log::OutS << "Max Directional Shadows Hit" << Log::OutS;
				lights[i]->SetShadow(false);
			}
		}
	}
	PipeLineState pipestate = PipeLineState{ true,false,false };
	pipestate.RenderTargetDesc = LightInteractions[0]->ShadowMap->GetPiplineRenderDesc();
	PointShadowList->SetPipelineState(pipestate);
	PointShadowList->SetPipelineStateObject(PointLightShader, LightInteractions[0]->ShadowMap);//all shadow buffers for cube maps are the same!

	PointShadowListALT->SetPipelineState(pipestate);
	PointShadowListALT->SetPipelineStateObject(PointLightShader, LightInteractions[0]->ShadowMap);//all shadow buffers for cube maps are the same!

	DirectionalShadowList->SetPipelineState(pipestate);
	DirectionalShadowList->SetPipelineStateObject(DirectionalLightShader, DirectionalLightBuffer);

}

void ShadowRenderer::Unbind(RHICommandList * list)
{
#if 0
	if (RHI::GetMGPUMode()->SplitShadowWork)
	{
		D3D12FrameBuffer* dBuffer = (D3D12FrameBuffer*)LightInteractions[2]->PreSampledBuffer;
		dBuffer->MakeReadyForCopy(((D3D12CommandList*)list)->GetCommandList());
	}
#endif
}

ShadowRenderer::ShadowLightInteraction::ShadowLightInteraction(DeviceContext * Context, bool IsPoint, int MapSize)
{
	DeviceIndex = Context->GetDeviceIndex();
	if (IsPoint)
	{
		int size = (Context->GetDeviceIndex() == 0) ? MapSize : MapSize / 1;
		RHIFrameBufferDesc desc = RHIFrameBufferDesc::CreateCubeDepth(size, size);
		//desc.DepthClearValue = 0.0f;
		ShadowMap = RHI::CreateFrameBuffer(Context, desc);
	}
	Shader = new Shader_Depth( Context, IsPoint);
	if (Context->GetDeviceIndex() != 0)
	{
		NeedsSample = true;
		RHIFrameBufferDesc desc = RHIFrameBufferDesc::CreateColourDepth(1264, 661);
		desc.IsShared = true;
		desc.DeviceToCopyTo = RHI::GetDeviceContext(0);
		desc.RTFormats[0] = eTEXTURE_FORMAT::FORMAT_R8_UNORM;
		PreSampledBuffer = RHI::CreateFrameBuffer(Context, desc);
	}
	IsPointLight = IsPoint;
}

ShadowRenderer::ShadowLightInteraction::~ShadowLightInteraction()
{
	SafeDelete(Shader)
	EnqueueSafeRHIRelease(ShadowMap);
	EnqueueSafeRHIRelease(PreSampledBuffer);
}

void ShadowRenderer::ShadowLightInteraction::PreSampleShadows(RHICommandList * List)
{

}
