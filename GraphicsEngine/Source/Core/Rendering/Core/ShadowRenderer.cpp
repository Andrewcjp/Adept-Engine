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
	if (RHI::GetMGPUSettings()->SFRSplitShadows || true)
	{
		DeviceZeroNeedsPreSample = true;
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
#if USE_GS_FOR_CUBE_SHADOWS
	DSOs[DeviceIndex].GeometryProjections->UpdateConstantBuffer(transforms, index);
#endif
}

#define USE32BITDepth 1
eTEXTURE_FORMAT ShadowRenderer::GetDepthType()
{
#if USE32BITDepth
	return eTEXTURE_FORMAT::FORMAT_D32_FLOAT;
#else
	return eTEXTURE_FORMAT::FORMAT_D16_UNORM;
#endif
}

eTEXTURE_FORMAT ShadowRenderer::GetDepthReadType()
{
#if USE32BITDepth
	return eTEXTURE_FORMAT::FORMAT_R32_FLOAT;
#else
	return eTEXTURE_FORMAT::FORMAT_R16_UNORM;
#endif
}

void ShadowRenderer::SetupOnDevice(DeviceContext * Context)
{
	if (Context == nullptr)
	{
		return;
	}
	DeviceShadowObjects* Objects = &DSOs[Context->GetDeviceIndex()];
	//	int shadowwidth = 1024;
	if (Context->GetDeviceIndex() == 0 || true)
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
	desc.RenderTargetDesc.DSVFormat = GetDepthType();
	Objects->ShadowPreSamplingList->SetPipelineStateDesc(desc);
	if (Context->GetDeviceIndex() == 1 || DeviceZeroNeedsPreSample)
	{
		if (Context->GetDeviceIndex() == 0)
		{
			InitPreSampled(Context, RHI::GetDeviceContext(1));
		}
		else
		{
			InitPreSampled(Context, RHI::GetDefaultDevice());
		}
	}
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
	if (con == nullptr)
	{
		return;
	}
	DeviceShadowObjects* Object = &DSOs[con->GetDeviceIndex()];
	const bool AsyncShadows = RHI::GetMGPUSettings()->AsyncShadows;
	const bool SplitShadows = RHI::GetMGPUSettings()->SplitShadowWork;
	if (RHI::GetFrameCount() > 0 && AsyncShadows && SplitShadows && con->GetDeviceIndex() > 0)
	{
		PreSampleShadows(Object->ShadowPreSamplingList, ShadowObjects);
		AsyncCopy(con->GetDeviceIndex());
	}

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
		if (RHI::GetMGPUSettings()->SplitShadowWork && !AsyncShadows || RHI::GetMGPUSettings()->SFRSplitShadows)
		{
			PreSampleShadows(Object->ShadowPreSamplingList, ShadowObjects);
		}
	}
}

void ShadowRenderer::RunPointShadowPass(RHICommandList * List, const std::vector<GameObject*>& ShadowObjects)
{
	List->ResetList();
	if (!RHI::GetMGPUSettings()->AsyncShadows || List->GetDeviceIndex() == 0)
	{
		List->GetDevice()->GetTimeManager()->StartTotalGPUTimer(List);
	}
	List->GetDevice()->GetTimeManager()->StartTimer(List, EGPUTIMERS::PointShadows);
	RenderPointShadows(List, ShadowObjects);
	List->GetDevice()->GetTimeManager()->EndTimer(List, EGPUTIMERS::PointShadows);
	if (RHI::GetMGPUSettings()->AsyncShadows && List->GetDeviceIndex() > 0)
	{
		List->GetDevice()->GetTimeManager()->EndTotalGPUTimer(List);
	}
	List->Execute();
}

void ShadowRenderer::AsyncCopy(int Index)
{
	if (DSOs[Index].PreSampledBuffer != nullptr)
	{
		if (RHI::GetMGPUSettings()->AsyncShadows)
		{
			FrameBuffer::CopyHelper(DSOs[Index].PreSampledBuffer, RHI::GetDeviceContext(0));
		}
	}
}

void ShadowRenderer::PreSampleShadows(RHICommandList* list, const std::vector<GameObject*>& ShadowObjects)
{
	SCOPE_CYCLE_COUNTER("PreSampleShadows");
	if (!NeedsCopyPreSample[list->GetDeviceIndex()])
	{
		return;
	}
	list->ResetList();
	if (RHI::GetMGPUSettings()->AsyncShadows && list->GetDeviceIndex() > 0)
	{
		list->GetDevice()->GetTimeManager()->StartTotalGPUTimer(list);
	}
	list->StartTimer(EGPUTIMERS::ShadowPreSample);
	//ensure(SampledLightInteractions < RHI::GetMGPUMode()->MAX_PRESAMPLED_SHADOWS);
	const int DeviceIndex = list->GetDeviceIndex();
	const int MaxShadow = RHI::GetMGPUSettings()->MAX_PRESAMPLED_SHADOWS;
	int ShaderData[4] = { 0 };
	int CurrnetIndex = 0;
	for (int i = 0; i < LightInteractions.size(); i++)
	{
		if (LightInteractions[i]->DeviceIndex != DeviceIndex)
		{
			continue;
		}
		if (!LightInteractions[i]->NeedsSample)
		{
			continue;
		}
		if (CurrnetIndex >= MaxShadow)
		{
			Log::LogMessage("Too many PreSampled Shadows Lights", Log::Severity::Error);
			break;
		}
		int id = -1;
		for (int x = 0; x < ShadowingPointLights.size(); x++)
		{
			if (ShadowingPointLights[x] == LightInteractions[i]->lightPtr)
			{
				id = x;
				break;
			}
		}
		if (id == -1)
		{
			continue;
		}
		ShaderData[CurrnetIndex] = id;
		CurrnetIndex++;
	}
	list->SetRootConstant(Shader_ShadowSample::PreSampleCBV, 4, ShaderData, 0);
	DSOs[DeviceIndex].ShadowCubeArray->BindToShader(list, Shader_ShadowSample::ShadowSRV);
	list->SetRenderTarget(DSOs[DeviceIndex].PreSampledBuffer);
	list->ClearFrameBuffer(DSOs[DeviceIndex].PreSampledBuffer);
	Scenerenderer->BindMvBuffer(list, Shader_Depth_RSSlots::VPBuffer);
	Scenerenderer->BindLightsBuffer(list, 1);
	for (size_t i = 0; i < ShadowObjects.size(); i++)
	{
		if (ShadowObjects[i]->GetMesh() == nullptr)
		{
			continue;
		}
		Scenerenderer->SetActiveIndex(list, (int)i, list->GetDeviceIndex());
		ShadowObjects[i]->Render(true, list);
	}

	list->SetRenderTarget(nullptr);

	list->EndTimer(EGPUTIMERS::ShadowPreSample);
	if (RHI::GetMGPUSettings()->SplitShadowWork && !RHI::GetMGPUSettings()->MainPassSFR && list->GetDeviceIndex() != 0 && !RHI::GetMGPUSettings()->AsyncShadows)
	{
		list->GetDevice()->GetTimeManager()->EndTotalGPUTimer(list);
	}
	if (list->GetDeviceIndex() == 1)
	{
		list->GetDevice()->InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::InterCopy);
	}
	list->Execute();

#if !SINGLE_GPU_PRESAMPLE
	if (DSOs[list->GetDeviceIndex()].PreSampledBuffer != nullptr)
	{
		if (!RHI::GetMGPUSettings()->AsyncShadows)
		{
			if (list->GetDeviceIndex() == 0)
			{
				if (NeedsCopyPreSample[0])
				{
					//SFR shadows is delaying all graphics work unitl copy complete point
					//FrameBuffer::CopyHelper(DSOs[list->GetDeviceIndex()].PreSampledBuffer, RHI::GetDeviceContext(1), EGPUCOPYTIMERS::ShadowCopy2);
					FrameBuffer::CopyHelper_NewSync(DSOs[list->GetDeviceIndex()].PreSampledBuffer, RHI::GetDeviceContext(1), EGPUCOPYTIMERS::ShadowCopy2);
				}
			}
			else
			{
				if (NeedsCopyPreSample[1])
				{
					FrameBuffer::CopyHelper(DSOs[list->GetDeviceIndex()].PreSampledBuffer, RHI::GetDeviceContext(0), EGPUCOPYTIMERS::ShadowCopy);
				}
			}
		}
	}
#endif

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
#if !USE_GS_FOR_CUBE_SHADOWS
		data.View = (glm::lookAtRH(data.Lightpos, data.Lightpos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0)));
#endif
		TargetShader->UpdateBuffer(list, &data, IndexOnGPU);
#if !USE_GS_FOR_CUBE_SHADOWS
		for (int Faces = 0; Faces < 6; Faces++)
		{
#endif
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
#if !USE_GS_FOR_CUBE_SHADOWS
		}
#endif
		IndexOnGPU++;
	}
	if (list->GetDeviceIndex() == 0 && RHI::GetMGPUSettings()->SplitShadowWork)
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
		if (RHI::GetMGPUSettings()->UseSplitShadows())
		{
			if (DSOs[1].PreSampledBuffer != nullptr && list->GetDeviceIndex() == 0)
			{
				list->SetFrameBufferTexture(DSOs[1].PreSampledBuffer, DeferredLightingShaderRSBinds::Limit);
			}
			if (DSOs[0].PreSampledBuffer != nullptr && list->GetDeviceIndex() == 1)
			{
				list->SetFrameBufferTexture(DSOs[0].PreSampledBuffer, DeferredLightingShaderRSBinds::Limit);
			}
		}
	}
	else
	{
		if (Object->ShadowCubeArray != nullptr)
		{
			//Object->ShadowDirectionalArray->BindToShader(list, MainShaderRSBinds::DirShadow);
			Object->ShadowCubeArray->BindToShader(list, MainShaderRSBinds::PointShadow);
		}
		if (RHI::GetMGPUSettings()->UseSplitShadows())
		{
			if (DSOs[1].PreSampledBuffer != nullptr && list->GetDeviceIndex() == 0)
			{
				list->SetFrameBufferTexture(DSOs[1].PreSampledBuffer, MainShaderRSBinds::PreSampledShadows);
			}
			if (DSOs[0].PreSampledBuffer != nullptr && list->GetDeviceIndex() == 1)
			{
				list->SetFrameBufferTexture(DSOs[0].PreSampledBuffer, MainShaderRSBinds::PreSampledShadows);
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
	if (RHI::GetMGPUSettings()->SplitShadowWork)
	{
		//ShadowingPointLights[0]->SetShadowResdent(1, 0);
		//ShadowingPointLights[1]->SetShadowResdent(1, 0);
		for (int i = 0; i < RHI::GetMGPUSettings()->MAX_PRESAMPLED_SHADOWS;i++)
		{
			ShadowingPointLights[i]->SetShadowResdent(1, 0);
		}
	}

	if (RHI::GetMGPUSettings()->MainPassSFR)
	{
		for (int l = 0; l < ShadowingPointLights.size(); l++)
		{
			for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
			{
				ShadowingPointLights[l]->GPUShadowResidentMask[i] = true;
			}
		}
	}

	if (RHI::GetMGPUSettings()->SFRSplitShadows)
	{
		ShadowingPointLights[0]->SetShadowResdent(1, 0);
		int ShadowsOnDev0 = RHI::GetMGPUSettings()->ShadowLightsOnDev1;
		for (int i = 0; i < ShadowsOnDev0; i++)
		{
			ShadowingPointLights[i]->SetShadowResdent(1, 0);
		}
		for (int i = ShadowsOnDev0; i < 4; i++)
		{
			ShadowingPointLights[i]->SetShadowResdent(0, 1);
		}
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
		if (i >= RHI::GetDeviceCount())
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
				NeedsCopyPreSample[i] = true;
			}
			sli->lightPtr = ShadowingPointLights[spli];
			LightInteractions.push_back(sli);
		}
		if (LightInteractions.size() > 0)
		{
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
}

void ShadowRenderer::Unbind(RHICommandList * list)
{
#if !SINGLE_GPU_PRESAMPLE
	if (RHI::GetMGPUSettings()->UseSplitShadows())
	{
		for (int i = 0; i < RHI::GetDeviceCount(); i++)
		{
			if (DSOs[i].PreSampledBuffer != nullptr && i != list->GetDeviceIndex())
			{
				DSOs[i].PreSampledBuffer->MakeReadyForCopy(list);
			}
		}
	}
#endif
}

ShadowRenderer::ShadowLightInteraction::ShadowLightInteraction(DeviceContext * Context, bool IsPoint, int MapSize)
{
	DevContext = Context;
	if (Context == nullptr)
	{
		return;
	}
	DeviceIndex = Context->GetDeviceIndex();
	if (IsPoint)
	{
		int size = (Context->GetDeviceIndex() == 0) ? MapSize : glm::iround(MapSize * RHI::GetMGPUSettings()->SecondCardShadowScaleFactor);
		RHIFrameBufferDesc desc = RHIFrameBufferDesc::CreateCubeDepth(size, size);
		desc.DepthFormat = ShadowRenderer::GetDepthType();
		desc.DepthReadFormat = ShadowRenderer::GetDepthReadType();
		ShadowMap = RHI::CreateFrameBuffer(Context, desc);
	}
	Shader = new Shader_Depth(Context, IsPoint);
	IsPointLight = IsPoint;
}

ShadowRenderer::ShadowLightInteraction::~ShadowLightInteraction()
{
	SafeDelete(Shader);
	EnqueueSafeRHIRelease(ShadowMap);
}

void ShadowRenderer::ShadowLightInteraction::SetupCopy(DeviceContext * TargetDev)
{
	NeedsSample = true;
}

void ShadowRenderer::InitPreSampled(DeviceContext* dev, DeviceContext* Targetdev)
{
	if (Targetdev == nullptr)
	{
		return;
	}
	if (DSOs[dev->GetDeviceIndex()].PreSampledBuffer != nullptr)
	{
		RHI::RemoveLinkedFrameBuffer(DSOs[dev->GetDeviceIndex()].PreSampledBuffer);
		EnqueueSafeRHIRelease(DSOs[dev->GetDeviceIndex()].PreSampledBuffer);
	}
	const int size = 512;
	RHIFrameBufferDesc desc = RHIFrameBufferDesc::CreateColourDepth(size, size);
	desc.IsShared = true;
	desc.DeviceToCopyTo = Targetdev;
	desc.RTFormats[0] = ShadowRenderer::GetPreSampledTextureFormat();
	desc.LinkToBackBufferScaleFactor = RHI::GetMGPUSettings()->PreSampleBufferScale;//todo: setting?
	DSOs[dev->GetDeviceIndex()].PreSampledBuffer = RHI::CreateFrameBuffer(dev, desc);
	RHI::AddLinkedFrameBuffer(DSOs[dev->GetDeviceIndex()].PreSampledBuffer);
}
eTEXTURE_FORMAT ShadowRenderer::GetPreSampledTextureFormat()
{
	const int MaxShadows = RHI::GetMGPUSettings()->MAX_PRESAMPLED_SHADOWS;
	if (MaxShadows == 1)
	{
		return eTEXTURE_FORMAT::FORMAT_R8_UNORM;
	}
	else if (MaxShadows == 2)
	{
		return eTEXTURE_FORMAT::FORMAT_R8G8_UNORM;
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
	EnqueueSafeRHIRelease(PreSampledBuffer);
}
