#include "ShadowRenderer.h"
#include "Core/GameObject.h"
#include "Core/Performance/PerfManager.h"
#include "Rendering/Shaders/MultiGPU/Shader_ShadowSample.h"
#include "Rendering/Shaders/Shader_Deferred.h"
#include "Rendering/Shaders/Shader_Depth.h"
#include "Rendering/Shaders/Shader_Main.h"
#include "RHI/DeviceContext.h"
#include "RHI/RHITypes.h"
#include "SceneRenderer.h"
#define GPU_SINGLE_PRESAMPLE 0
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
	for (int i = 0; i < RHI::GetRenderConstants()->MAX_DYNAMIC_DIRECTIONAL_SHADOWS; i++)
	{
		//ShadowCubeArray->SetIndexNull(i);
	}
#if !TEST_PRESAMPLE
	for (int i = 0; i < MAX_POINT_SHADOWS; i++)
	{
		PointLightBuffers.push_back(RHI::CreateFrameBuffer(RHI::GetDeviceContext(0), RHIFrameBufferDesc::CreateCubeDepth(shadowwidth, shadowwidth)));
		ShadowCubeArray->AddFrameBufferBind(PointLightBuffers[i], i);
	}
#else

#if GPU_SINGLE_PRESAMPLE
	DeviceContext* AltDevice = RHI::GetDeviceContext(0);
#else 
	DeviceContext* AltDevice = RHI::GetDeviceContext(1);
#endif
#if 0
	const int ShadowMapSize = RHI::GetRenderSettings()->ShadowMapSize;
	for (int i = 0; i < RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS; i++)
	{
		if ((i == 2 /*|| i== 3*/) && RHI::GetMGPUMode()->SplitShadowWork)
		{
			LightInteractions.push_back(new ShadowLightInteraction(AltDevice, true, ShadowMapSize));
			ShadowCubeArray->SetIndexNull(2);
		}
		else
		{
			LightInteractions.push_back(new ShadowLightInteraction(RHI::GetDeviceContext(0), true, ShadowMapSize));
			ShadowCubeArray->AddFrameBufferBind(LightInteractions[i]->ShadowMap, i);
		}
	}
#endif
#endif
	DeviceContext* pointlightdevice = RHI::GetDeviceContext(0);
	if (RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS > 0)
	{
		PointLightShader = ShaderComplier::GetShader<Shader_Depth>(pointlightdevice, true);
		GeometryProjections = RHI::CreateRHIBuffer(ERHIBufferType::Constant, pointlightdevice);
		GeometryProjections->CreateConstantBuffer(sizeof(glm::mat4) * CUBE_SIDES, RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS, true);
		PointShadowList = RHI::CreateCommandList(ECommandListType::Graphics, pointlightdevice);
		PointShadowListALT = RHI::CreateCommandList(ECommandListType::Graphics, AltDevice);
		NAME_RHI_OBJECT(PointShadowListALT);
		NAME_RHI_OBJECT(PointShadowList);
	}
	DirectionalShadowList = RHI::CreateCommandList();
	NAME_RHI_OBJECT(DirectionalShadowList);
#if TEST_PRESAMPLE
	ShadowPreSampleShader = ShaderComplier::GetShader<Shader_ShadowSample>(pointlightdevice);
	ShadowPreSamplingList = RHI::CreateCommandList(ECommandListType::Graphics, AltDevice);
	NAME_RHI_OBJECT(ShadowPreSamplingList);
	//if (LightInteractions.size() > 2)
	{
		//ShadowPreSamplingList->CreatePipelineState(ShadowPreSampleShader, LightInteractions[2]->PreSampledBuffer);
		RHIPipeLineStateDesc desc;
		desc.ShaderInUse = ShadowPreSampleShader;
		desc.RenderTargetDesc.NumRenderTargets = 1;
		desc.RenderTargetDesc.RTVFormats[0] = eTEXTURE_FORMAT::FORMAT_R8_UNORM;
		desc.RenderTargetDesc.DSVFormat = eTEXTURE_FORMAT::FORMAT_D32_FLOAT;
		//desc.FrameBufferTarget = LightInteractions[0]->PreSampledBuffer;
		ShadowPreSamplingList->SetPipelineStateDesc(desc);

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
		int Constant = SNum;
		list->SetRootConstant(Shader_ShadowSample::PreSampleCBV, 1, &Constant, 0);
		list->SetFrameBufferTexture(LightInteractions[SNum]->ShadowMap, Shader_ShadowSample::ShadowSRV);
		list->SetRenderTarget(LightInteractions[SNum]->PreSampledBuffer);
		list->ClearFrameBuffer(LightInteractions[SNum]->PreSampledBuffer);
		Scenerenderer->BindMvBuffer(list, Shader_Depth_RSSlots::VPBuffer);
		Scenerenderer->BindLightsBuffer(list, 1);
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
#if !GPU_SINGLE_PRESAMPLE
		if (RHI::GetMGPUMode()->AsyncShadows)
		{
			FrameBuffer::CopyHelper_Async_OneFrame(LightInteractions[SNum]->PreSampledBuffer, RHI::GetDeviceContext(0));
		}
		else
		{
			FrameBuffer::CopyHelper(LightInteractions[SNum]->PreSampledBuffer, RHI::GetDeviceContext(0));
		}
#endif
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
	//#else
	if (RHI::GetMGPUMode()->SplitShadowWork)
	{
#if 1
		for (int i = 0; i < LightInteractions.size(); i++)
		{
			if (LightInteractions[i]->lightPtr->ExecOnAlt)
			{
				list->SetFrameBufferTexture(LightInteractions[i]->PreSampledBuffer, DeferredLightingShaderRSBinds::Limit);
			}
		}
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
	if (RHI::GetMGPUMode()->SplitShadowWork)
	{
		ShadowingPointLights[0]->ExecOnAlt = true;
	}
	//testing
	ShadowCubeArray->Clear();//removes all refs to any buffer we had last frame!
	MemoryUtils::DeleteVector(LightInteractions);

	const int ShadowMapSize = RHI::GetRenderSettings()->ShadowMapSize;
	for (int i = 0; i < ShadowingPointLights.size(); i++)
	{
		ShadowLightInteraction* sli = new ShadowLightInteraction(RHI::GetDeviceContext(ShadowingPointLights[i]->ExecOnAlt ? 1 : 0), true, ShadowMapSize);
		sli->lightPtr = ShadowingPointLights[i];
		if (sli->lightPtr->ExecOnAlt)
		{
			ShadowCubeArray->SetIndexNull(i, sli->ShadowMap);
		}
		else
		{
			ShadowCubeArray->AddFrameBufferBind(sli->ShadowMap, i);
		}
		LightInteractions.push_back(sli);
	}

	RHIPipeLineStateDesc desc;
	desc.InitOLD(true, false, false);
	desc.RenderTargetDesc = LightInteractions[0]->ShadowMap->GetPiplineRenderDesc();
	desc.ShaderInUse = PointLightShader;
	desc.FrameBufferTarget = LightInteractions[0]->ShadowMap;
	PointShadowList->SetPipelineStateDesc(desc);
	PointShadowListALT->SetPipelineStateDesc(desc);
	desc.ShaderInUse = DirectionalLightShader;
	desc.FrameBufferTarget = DirectionalLightBuffer;
	DirectionalShadowList->SetPipelineStateDesc(desc);
}

void ShadowRenderer::Unbind(RHICommandList * list)
{
#if !GPU_SINGLE_PRESAMPLE
	if (RHI::GetMGPUMode()->SplitShadowWork)
	{
		for (int i = 0; i < LightInteractions.size(); i++)
		{
			if (LightInteractions[i]->lightPtr->ExecOnAlt)
			{
				LightInteractions[i]->PreSampledBuffer->MakeReadyForCopy(list);
			}
		}
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
		ShadowMap = RHI::CreateFrameBuffer(Context, desc);
	}
	Shader = new Shader_Depth(Context, IsPoint);
#if !GPU_SINGLE_PRESAMPLE
	if (Context->GetDeviceIndex() != 0)
#endif
	{
		NeedsSample = true;
#if GPU_SINGLE_PRESAMPLE
		RHIFrameBufferDesc desc = RHIFrameBufferDesc::CreateColourDepth(2048, 661);
		desc.RTFormats[0] = eTEXTURE_FORMAT::FORMAT_R8_UNORM;
		PreSampledBuffer = RHI::CreateFrameBuffer(Context, desc);
#else
		const int size = 2048;
		RHIFrameBufferDesc desc = RHIFrameBufferDesc::CreateColourDepth(size, size);
		desc.IsShared = true;
		desc.DeviceToCopyTo = RHI::GetDeviceContext(0);
		desc.RTFormats[0] = eTEXTURE_FORMAT::FORMAT_R8_UNORM;
		PreSampledBuffer = RHI::CreateFrameBuffer(Context, desc);
#endif
	}
	IsPointLight = IsPoint;
}

ShadowRenderer::ShadowLightInteraction::~ShadowLightInteraction()
{
	SafeDelete(Shader);
	EnqueueSafeRHIRelease(ShadowMap);
	EnqueueSafeRHIRelease(PreSampledBuffer);
}
