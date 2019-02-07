#include "ShadowRenderer.h"
#include "Core/GameObject.h"
#include "Core/Performance/PerfManager.h"
#include "Rendering/Shaders/MultiGPU/Shader_ShadowSample.h"
#include "Rendering/Shaders/Shader_Deferred.h"
#include "Rendering/Shaders/Shader_Depth.h"
#include "Rendering/Shaders/Shader_Main.h"
#include "RHI/DeviceContext.h"
#include "SceneRenderer.h"
#include "Core/Platform/PlatformCore.h"
#define SINGLE_GPU_PRESAMPLE 0
#define CUBE_SIDES 6
#define TEST_PRESAMPLE 1
ShadowRenderer::ShadowRenderer(SceneRenderer * sceneRenderer)
{
	Scenerenderer = sceneRenderer;
	DirectionalLightShader = ShaderComplier::GetShader_Default<Shader_Depth>(false);
	if (RHI::GetMGPUMode()->MainPassSFR)
	{
		AllDevicesNeedToRead = true;
	}
	PointLightShader = ShaderComplier::GetShader<Shader_Depth>(RHI::GetDefaultDevice(), true);
	ShadowPreSampleShader = ShaderComplier::GetShader<Shader_ShadowSample>();
	SetupOnDevice(RHI::GetDeviceContext(0));
	SetupOnDevice(RHI::GetDeviceContext(1));
}

ShadowRenderer::~ShadowRenderer()
{
	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		DSOs[i].Release();
	}
	MemoryUtils::RHIUtil::DeleteVector(DirectionalLightBuffers);
	MemoryUtils::DeleteVector(LightInteractions);
}

void ShadowRenderer::UpdateGeometryShaderParams(glm::vec3 lightPos, glm::mat4 shadowProj, int index, int DeviceIndex)
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

	DSOs[DeviceIndex].GeometryProjections->UpdateConstantBuffer(transforms, index);
}

void ShadowRenderer::SetupOnDevice(DeviceContext * Context)
{
	if (Context == nullptr)
	{
		return;
	}
	DeviceShadowObjects* Objects = &DSOs[Context->GetDeviceIndex()];
	//	int shadowwidth = 1024;
	if (Context->GetDeviceIndex() == 0 || AllDevicesNeedToRead)
	{
		Objects->ShadowDirectionalArray = RHI::CreateTextureArray(Context, RHI::GetRenderConstants()->MAX_DYNAMIC_DIRECTIONAL_SHADOWS);
		//for (int i = 0; i < RHI::GetRenderConstants()->MAX_DYNAMIC_DIRECTIONAL_SHADOWS; i++)
		//{
		//	DirectionalLightBuffers.push_back(RHI::CreateFrameBuffer(Context, RHIFrameBufferDesc::CreateDepth(shadowwidth, shadowwidth)));
		//	Objects->ShadowDirectionalArray->AddFrameBufferBind(DirectionalLightBuffers[i], i);
		//}
		Objects->ShadowCubeArray = RHI::CreateTextureArray(Context, RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS);
	}
	if (RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS > 0)
	{
		Objects->GeometryProjections = RHI::CreateRHIBuffer(ERHIBufferType::Constant, Context);
		Objects->GeometryProjections->CreateConstantBuffer(sizeof(glm::mat4) * CUBE_SIDES, RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS, false);
		Objects->PointLightShadowList = RHI::CreateCommandList(ECommandListType::Graphics, Context);
		NAME_RHI_OBJECT(Objects->PointLightShadowList);
	}

	Objects->DirectionalShadowList = RHI::CreateCommandList(ECommandListType::Graphics, Context);
	NAME_RHI_OBJECT(Objects->DirectionalShadowList);
	Objects->ShadowPreSamplingList = RHI::CreateCommandList(ECommandListType::Graphics, Context);
	NAME_RHI_OBJECT(Objects->ShadowPreSamplingList);

	RHIPipeLineStateDesc desc;
	desc.ShaderInUse = ShadowPreSampleShader;
	desc.RenderTargetDesc.NumRenderTargets = 1;
	desc.RenderTargetDesc.RTVFormats[0] = GetPreSampledTextureFormat();
	desc.RenderTargetDesc.DSVFormat = eTEXTURE_FORMAT::FORMAT_D32_FLOAT;
	Objects->ShadowPreSamplingList->SetPipelineStateDesc(desc);
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
	RenderOnDevice(RHI::GetDeviceContext(0), ShadowObjects);
	RenderOnDevice(RHI::GetDeviceContext(1), ShadowObjects);
}

void ShadowRenderer::RenderOnDevice(DeviceContext* con, const std::vector<GameObject*>& ShadowObjects)
{
	DeviceShadowObjects* Object = &DSOs[con->GetDeviceIndex()];
	if (ShadowingDirectionalLights.size() > 0 && false)
	{
		Object->DirectionalShadowList->ResetList();
		Object->DirectionalShadowList->GetDevice()->GetTimeManager()->StartTotalGPUTimer(Object->DirectionalShadowList);
		Object->DirectionalShadowList->GetDevice()->GetTimeManager()->StartTimer(Object->DirectionalShadowList, EGPUTIMERS::DirShadows);
		RenderDirectionalShadows(Object->DirectionalShadowList, ShadowObjects);
		Object->DirectionalShadowList->GetDevice()->GetTimeManager()->EndTimer(Object->DirectionalShadowList, EGPUTIMERS::DirShadows);
		Object->DirectionalShadowList->Execute();
	}
	if (ShadowingPointLights.size() > 0)
	{
		RunPointShadowPass(Object->PointLightShadowList, ShadowObjects);
		if (RHI::GetMGPUMode()->SplitShadowWork)
		{
			PreSampleShadows(Object->ShadowPreSamplingList, ShadowObjects);
		}
	}
}

void ShadowRenderer::RunPointShadowPass(RHICommandList * List, const std::vector<GameObject*>& ShadowObjects)
{
	List->ResetList();
	List->GetDevice()->GetTimeManager()->StartTotalGPUTimer(List);
	List->GetDevice()->GetTimeManager()->StartTimer(List, EGPUTIMERS::PointShadows);
	RenderPointShadows(List, ShadowObjects);
	List->GetDevice()->GetTimeManager()->EndTimer(List, EGPUTIMERS::PointShadows);
	List->Execute();
}

void ShadowRenderer::PreSampleShadows(RHICommandList* list, const std::vector<GameObject*>& ShadowObjects)
{
	SCOPE_CYCLE_COUNTER("PreSampleShadows");
	list->ResetList();
	list->StartTimer(EGPUTIMERS::ShadowPreSample);
	int SampledLightInteractions = 0;
	for (int SNum = 0; SNum < (int)LightInteractions.size(); SNum++)
	{
		if (!LightInteractions[SNum]->NeedsSample)
		{
			continue;
		}
		if (LightInteractions[SNum]->DeviceIndex != list->GetDeviceIndex())
		{
			continue;
		}
		ensure(SampledLightInteractions < RHI::GetMGPUMode()->MAX_PRESAMPLED_SHADOWS);
		SampledLightInteractions++;
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
	if (RHI::GetMGPUMode()->SplitShadowWork && !RHI::GetMGPUMode()->MainPassSFR && list->GetDeviceIndex() != 0)
	{
		list->GetDevice()->GetTimeManager()->EndTotalGPUTimer(list);
	}
	list->Execute();
	for (int SNum = 0; SNum < (int)LightInteractions.size(); SNum++)
	{
		if (!LightInteractions[SNum]->NeedsSample)
		{
			continue;
		}
		if (LightInteractions[SNum]->DeviceIndex != list->GetDeviceIndex())
		{
			continue;
		}
#if !SINGLE_GPU_PRESAMPLE
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

void ShadowRenderer::RenderPointShadows(RHICommandList * list, const std::vector<GameObject*>& ShadowObjects)
{
	int IndexOnGPU = 0;
	for (int SNum = 0; SNum < (int)LightInteractions.size(); SNum++)
	{
		if (!LightInteractions[SNum]->IsPointLight)
		{
			continue;
		}
		if (list->GetDeviceIndex() != LightInteractions[SNum]->DeviceIndex && !LightInteractions[SNum]->SampleOnAllDevices)
		{
			continue;
		}
		FrameBuffer* TargetBuffer = LightInteractions[SNum]->ShadowMap;
		Shader_Depth* TargetShader = LightInteractions[SNum]->Shader;
		Light* LightPtr = LightInteractions[SNum]->lightPtr;
		list->SetRenderTarget(TargetBuffer);
		list->ClearFrameBuffer(TargetBuffer);
		UpdateGeometryShaderParams(LightPtr->GetPosition(), LightPtr->Projection, IndexOnGPU, list->GetDeviceIndex());
		list->SetConstantBufferView(DSOs[list->GetDeviceIndex()].GeometryProjections, IndexOnGPU, Shader_Depth_RSSlots::GeometryProjections);
		Shader_Depth::LightData data = {};
		data.Proj = LightPtr->Projection;
		data.Lightpos = LightPtr->GetPosition();
		TargetShader->UpdateBuffer(list, &data, IndexOnGPU);
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
		IndexOnGPU++;
	}
	if (list->GetDeviceIndex() == 0)
	{
		list->InsertGPUStallTimer();
	}
}

void ShadowRenderer::RenderDirectionalShadows(RHICommandList * list, const std::vector<GameObject *> & ShadowObjects)
{
	for (size_t SNum = 0; SNum < ShadowingDirectionalLights.size(); SNum++)
	{
		FrameBuffer* TargetBuffer = DirectionalLightBuffers[SNum];
		list->SetRenderTarget(TargetBuffer);
		list->ClearFrameBuffer(TargetBuffer);
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
			ShadowObjects[i]->Render(true, list);
		}
		list->SetRenderTarget(nullptr);
	}
}

void ShadowRenderer::BindShadowMapsToTextures(RHICommandList * list)
{
	DeviceShadowObjects* Object = &DSOs[list->GetDeviceIndex()];
	if (RHI::GetRenderSettings()->IsDeferred)
	{
		//Object->ShadowDirectionalArray->BindToShader(list, 5);
		Object->ShadowCubeArray->BindToShader(list, 6);
	}
	else
	{
		if (Object->ShadowCubeArray != nullptr)
		{
			//Object->ShadowDirectionalArray->BindToShader(list, MainShaderRSBinds::DirShadow);
			Object->ShadowCubeArray->BindToShader(list, MainShaderRSBinds::PointShadow);
		}
	}

	if (RHI::GetMGPUMode()->SplitShadowWork)
	{
		for (int i = 0; i < LightInteractions.size(); i++)
		{
			if (LightInteractions[i]->PreSampledBuffer != nullptr)
			{
				list->SetFrameBufferTexture(LightInteractions[i]->PreSampledBuffer, DeferredLightingShaderRSBinds::Limit);
			}
		}
	}
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
		//ShadowingPointLights[1]->GPUShadowResidentMask[1] = true;
		//ShadowingPointLights[1]->GPUShadowCopyDeviceTarget[1] = 0;
		ShadowingPointLights[0]->GPUShadowResidentMask[1] = true;
		ShadowingPointLights[0]->GPUShadowCopyDeviceTarget[1] = 0;
	}

	//removes all refs to any buffer we had last frame!
	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		if (DSOs[i].ShadowCubeArray)
		{
			DSOs[i].ShadowCubeArray->Clear();
		}
	}
	MemoryUtils::DeleteVector(LightInteractions);

	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		if (i > RHI::GetDeviceCount())
		{
			continue;
		}
		const int ShadowMapSize = RHI::GetRenderSettings()->ShadowMapSize;
		for (int spli = 0; spli < ShadowingPointLights.size(); spli++)
		{
			ShadowLightInteraction* sli = nullptr;
			sli = new ShadowLightInteraction(RHI::GetDeviceContext(i), true, ShadowMapSize);
			if (!ShadowingPointLights[spli]->GPUShadowResidentMask[i])
			{
				if (DSOs[i].ShadowCubeArray != nullptr)
				{
					DSOs[i].ShadowCubeArray->SetIndexNull(spli, sli->ShadowMap);
				}
				SafeDelete(sli);
				continue;
			}
			if (DSOs[i].ShadowCubeArray != nullptr)
			{
				if (ShadowingPointLights[spli]->GPUShadowResidentMask[i])
				{
					DSOs[i].ShadowCubeArray->AddFrameBufferBind(sli->ShadowMap, spli);
				}
				else
				{
					DSOs[i].ShadowCubeArray->SetIndexNull(spli, sli->ShadowMap);
				}
			}
			const int Device_CopyTarget = ShadowingPointLights[spli]->GPUShadowCopyDeviceTarget[i];
			if (Device_CopyTarget != -1 && Device_CopyTarget != i)//Set and not Current device
			{
				sli->SetupCopy(RHI::GetDeviceContext(Device_CopyTarget));
			}
			sli->lightPtr = ShadowingPointLights[spli];
			LightInteractions.push_back(sli);
		}

		RHIPipeLineStateDesc desc;
		desc.InitOLD(true, false, false);
		desc.RenderTargetDesc = LightInteractions[0]->ShadowMap->GetPiplineRenderDesc();
		desc.ShaderInUse = PointLightShader;
		desc.FrameBufferTarget = LightInteractions[0]->ShadowMap;
		DSOs[i].PointLightShadowList->SetPipelineStateDesc(desc);
		desc.ShaderInUse = DirectionalLightShader;
		desc.FrameBufferTarget = DirectionalLightBuffer;
		DSOs[i].DirectionalShadowList->SetPipelineStateDesc(desc);
	}
}

void ShadowRenderer::Unbind(RHICommandList * list)
{
#if !SINGLE_GPU_PRESAMPLE
	if (RHI::GetMGPUMode()->SplitShadowWork)
	{
		for (int i = 0; i < LightInteractions.size(); i++)
		{
			if (LightInteractions[i]->PreSampledBuffer != nullptr/* && LightInteractions[i]->DeviceIndex == list->GetDeviceIndex()*/)
			{
				LightInteractions[i]->PreSampledBuffer->MakeReadyForCopy(list);
			}
		}
	}
#endif
}

ShadowRenderer::ShadowLightInteraction::ShadowLightInteraction(DeviceContext * Context, bool IsPoint, int MapSize)
{
	DevContext = Context;
	DeviceIndex = Context->GetDeviceIndex();
	if (IsPoint)
	{
		int size = (Context->GetDeviceIndex() == 0) ? MapSize : MapSize / 1;
		RHIFrameBufferDesc desc = RHIFrameBufferDesc::CreateCubeDepth(size, size);
		ShadowMap = RHI::CreateFrameBuffer(Context, desc);
	}
	Shader = new Shader_Depth(Context, IsPoint);
	IsPointLight = IsPoint;
}

ShadowRenderer::ShadowLightInteraction::~ShadowLightInteraction()
{
	SafeDelete(Shader);
	EnqueueSafeRHIRelease(ShadowMap);
	EnqueueSafeRHIRelease(PreSampledBuffer);
}

void ShadowRenderer::ShadowLightInteraction::SetupCopy(DeviceContext * TargetDev)
{
#if !SINGLE_GPU_PRESAMPLE
	if (DevContext->GetDeviceIndex() != 0)
#endif
	{
		NeedsSample = true;
#if SINGLE_GPU_PRESAMPLE
		RHIFrameBufferDesc desc = RHIFrameBufferDesc::CreateColourDepth(2048, 661);
		desc.RTFormats[0] = eTEXTURE_FORMAT::FORMAT_R8_UNORM;
		PreSampledBuffer = RHI::CreateFrameBuffer(Context, desc);
#else
		const int size = 512;
		RHIFrameBufferDesc desc = RHIFrameBufferDesc::CreateColourDepth(size, size);
		desc.IsShared = true;
		desc.DeviceToCopyTo = RHI::GetDeviceContext(0);
		desc.RTFormats[0] = ShadowRenderer::GetPreSampledTextureFormat();
		PreSampledBuffer = RHI::CreateFrameBuffer(DevContext, desc);
#endif
	}
}

eTEXTURE_FORMAT ShadowRenderer::GetPreSampledTextureFormat()
{
	const int MaxShadows = RHI::GetMGPUMode()->MAX_PRESAMPLED_SHADOWS;
	if (MaxShadows == 1)
	{
		return eTEXTURE_FORMAT::FORMAT_R8_UNORM;
	}
	else if (MaxShadows == 2)
	{
		return  eTEXTURE_FORMAT::FORMAT_R8G8_UNORM;
	}
	//todo: don't copy alpha channel of 3 lights !
	return eTEXTURE_FORMAT::FORMAT_R8G8B8A8_UNORM;
}

bool ShadowRenderer::ShadowLightInteraction::IsResident(DeviceContext * dev) const
{
	return lightPtr->GPUShadowResidentMask[dev->GetDeviceIndex()];
}

void DeviceShadowObjects::Release()
{
	EnqueueSafeRHIRelease(GeometryProjections);
	EnqueueSafeRHIRelease(ShadowCubeArray);
	EnqueueSafeRHIRelease(ShadowDirectionalArray);
	EnqueueSafeRHIRelease(PointLightShadowList);
	EnqueueSafeRHIRelease(DirectionalShadowList);
	EnqueueSafeRHIRelease(ShadowPreSamplingList);

}
