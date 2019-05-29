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
#include "Mesh/MeshPipelineController.h"
#include "Culling/CullingManager.h"
#define SINGLE_GPU_PRESAMPLE 0
#define CUBE_SIDES 6
#define TEST_PRESAMPLE 1

ShadowRenderer* ShadowRenderer::Instance = nullptr;

ShadowRenderer::ShadowRenderer(SceneRenderer * sceneRenderer, CullingManager* manager)
{
	ViewInstancesPerDraw = 3;
	Instance = this;
	Scenerenderer = sceneRenderer;
	DirectionalLightShader = ShaderComplier::GetShader_Default<Shader_Depth>(false);
	if (RHI::GetMGPUSettings()->SFRSplitShadows || true)
	{
		DeviceZeroNeedsPreSample = true;
	}
	PointLightShader = ShaderComplier::GetShader<Shader_Depth>(RHI::GetDefaultDevice(), true);
	ShadowPreSampleShader = ShaderComplier::GetShader_Default<Shader_ShadowSample, int>(RHI::GetMGPUSettings()->MAX_PRESAMPLED_SHADOWS);
	ShadowPreSampleShader_GPU0 = ShaderComplier::GetShader_Default<Shader_ShadowSample, int>(RHI::GetMGPUSettings()->MAX_PRESAMPLED_SHADOWS_GPU0);
	Culling = manager;
	for (int i = 0; i < RHI::GetDeviceCount(); i++)
	{
		SetupOnDevice(RHI::GetDeviceContext(i));
	}

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
		Objects->ShadowCubeArray->SetFrameBufferFormat(GetCubeMapFBDesc());
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
	if (Context->GetDeviceIndex() == 0)
	{
		desc.ShaderInUse = ShadowPreSampleShader_GPU0;
	}
	else
	{
		desc.ShaderInUse = ShadowPreSampleShader;
	}
	desc.RenderTargetDesc.NumRenderTargets = 1;
	desc.RenderTargetDesc.RTVFormats[0] = GetPreSampledTextureFormat(Context->GetDeviceIndex());
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

PreSampleController * ShadowRenderer::GetSampleController(int index)
{
	return &Instance->SampleControllers[index];
}


void ShadowRenderer::RenderShadowMaps(Camera * c, std::vector<Light*>& lights, const std::vector<GameObject*>& ShadowObjects, Shader_Main* mainshader)
{
	SCOPE_CYCLE_COUNTER_GROUP("Shadow CPU", "Render");
	if (UseCache)
	{
		if (Renderered)
		{
			return;
		}
		Renderered = true;
	}
	if (!NeedsAnyShadowUpdate())
	{
		return;
	}
	RenderOnDevice(RHI::GetDeviceContext(0), ShadowObjects);
	RenderOnDevice(RHI::GetDeviceContext(1), ShadowObjects);
}

bool ShadowRenderer::NeedsAnyShadowUpdate()
{
	for (int i = 0; i < LightInteractions.size(); i++)
	{
		if (LightInteractions[i]->NeedsUpdate())
		{
			return true;
		}
	}
	return false;
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
	int MaxShadow = RHI::GetMGPUSettings()->MAX_PRESAMPLED_SHADOWS;
	if (DeviceIndex == 0)
	{
		MaxShadow = RHI::GetMGPUSettings()->MAX_PRESAMPLED_SHADOWS_GPU0;
	}
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
	Scenerenderer->Controller->RenderPass(ERenderPass::DepthOnly, list, nullptr);
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
		if (!LightInteractions[SNum]->lightPtr->GetType() == ELightType::Point)
		{
			continue;
		}
		ShadowLightInteraction* Interaction = LightInteractions[SNum];
		if (!Interaction->NeedsRenderOnDevice(list->GetDevice()))
		{
			continue;
		}
		if (!Interaction->NeedsUpdate())
		{
			continue;
		}
		if (RHI::GetRenderSettings()->UseGeometryShaderForShadows)
		{
			RenderShadowMap_GPU(Interaction, list, IndexOnGPU);
		}
		else
		{
			RenderShadowMap_CPU(Interaction, list, IndexOnGPU);
		}
		Interaction->captured = true;
		IndexOnGPU++;
	}
	if (list->GetDeviceIndex() == 0 && RHI::GetMGPUSettings()->SplitShadowWork)
	{
		list->InsertGPUStallTimer();
	}
}

void ShadowRenderer::RenderShadowMap_GPU(ShadowLightInteraction* Interaction, RHICommandList * list, int IndexOnGPU)
{
	Culling->UpdateCullingForShadowLight(Interaction->lightPtr, Scenerenderer->GetScene());

	FrameBuffer* TargetBuffer = Interaction->GetMap(list->GetDeviceIndex());
	Shader_Depth* TargetShader = Interaction->Shader;
	Light* LightPtr = Interaction->lightPtr;
	list->BeginRenderPass(RHIRenderPassInfo(TargetBuffer, ERenderPassLoadOp::Clear));
	UpdateGeometryShaderParams(LightPtr->GetPosition(), LightPtr->Projection, IndexOnGPU, list->GetDeviceIndex());
	list->SetConstantBufferView(DSOs[list->GetDeviceIndex()].GeometryProjections, IndexOnGPU, Shader_Depth_RSSlots::GeometryProjections);
	Shader_Depth::LightData data = {};
	data.Proj = LightPtr->Projection;
	data.Lightpos = LightPtr->GetPosition();
	TargetShader->UpdateBuffer(list, &data, IndexOnGPU);
	list->SetSingleRootConstant(Shader_Depth_RSSlots::VI_Offset, 0);
	EBatchFilter::Type Filter = EBatchFilter::ALL;
	if (LightPtr->GetShadowMode() == EShadowCaptureType::Baked || LightPtr->GetShadowMode() == EShadowCaptureType::Stationary)
	{
		Filter = EBatchFilter::StaticOnly;
	}
	if (RHI::GetRenderSettings()->UseViewInstancing)
	{
		for (int i = 0; i < CUBE_SIDES; i += ViewInstancesPerDraw)
		{
			list->SetSingleRootConstant(Shader_Depth_RSSlots::VI_Offset, i);
			Scenerenderer->Controller->RenderPass(ERenderPass::DepthOnly, list, TargetShader, Filter);
		}
	}
	else
	{
		Scenerenderer->Controller->RenderPass(ERenderPass::DepthOnly, list, TargetShader, Filter);
	}
	list->EndRenderPass();
}

void ShadowRenderer::RenderShadowMap_CPU(ShadowLightInteraction * Interaction, RHICommandList * list, int IndexOnGPU)
{
	Culling->UpdateCullingForShadowLight(Interaction->lightPtr, Scenerenderer->GetScene());

	FrameBuffer* TargetBuffer = Interaction->GetMap(list->GetDeviceIndex());
	Shader_Depth* TargetShader = Interaction->Shader;
	Light* LightPtr = Interaction->lightPtr;
	list->BeginRenderPass(RHIRenderPassInfo(TargetBuffer, ERenderPassLoadOp::Clear));
	UpdateGeometryShaderParams(LightPtr->GetPosition(), LightPtr->Projection, IndexOnGPU, list->GetDeviceIndex());
	list->SetConstantBufferView(DSOs[list->GetDeviceIndex()].GeometryProjections, IndexOnGPU, Shader_Depth_RSSlots::GeometryProjections);
	Shader_Depth::LightData data = {};
	data.Proj = LightPtr->Projection;
	data.Lightpos = LightPtr->GetPosition();
	TargetShader->UpdateBuffer(list, &data, IndexOnGPU);
	for (int Faces = 0; Faces < 6; Faces++)
	{
		list->SetSingleRootConstant(Shader_Depth_RSSlots::VI_Offset, Faces);
		Scenerenderer->Controller->RenderPass(ERenderPass::DepthOnly, list, TargetShader);
	}
	list->EndRenderPass();
}

void ShadowRenderer::RenderDirectionalShadows(RHICommandList * list, const std::vector<GameObject *> & ShadowObjects)
{
	for (int SNum = 0; SNum < (int)LightInteractions.size(); SNum++)
	{
		if (!LightInteractions[SNum]->lightPtr->GetType() == ELightType::Directional)
		{
			continue;
		}
		FrameBuffer* TargetBuffer = LightInteractions[SNum]->GetMap(list->GetDeviceIndex());
		list->SetRenderTarget(TargetBuffer);
		list->ClearFrameBuffer(TargetBuffer);
		Shader_Depth::LightData data = {};
		data.Proj = ShadowingDirectionalLights[SNum]->Projection;
		data.Lightpos = ShadowingDirectionalLights[SNum]->GetPosition();
		DirectionalLightShader->UpdateBuffer(list, &data, (int)SNum);
		Scenerenderer->Controller->RenderPass(ERenderPass::DepthOnly, list, LightInteractions[SNum]->Shader);
		list->SetRenderTarget(nullptr);
	}
}

void ShadowRenderer::BindShadowMapsToTextures(RHICommandList * list, bool cubemap)
{
	DeviceShadowObjects* Object = &DSOs[list->GetDeviceIndex()];
	if (RHI::GetRenderSettings()->IsDeferred && !cubemap)
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

			//list->SetFrameBufferTexture(LightInteractions[0]->ShadowMap, MainShaderRSBinds::PointShadow);
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
RHIPipeRenderTargetDesc ShadowRenderer::GetCubeMapDesc()
{
	return GetCubeMapFBDesc(1).GetRTDesc();
}
RHIFrameBufferDesc ShadowRenderer::GetCubeMapFBDesc(int size)
{
	RHIFrameBufferDesc desc = RHIFrameBufferDesc::CreateCubeDepth(size, size);
	desc.DepthFormat = ShadowRenderer::GetDepthType();
	desc.DepthReadFormat = ShadowRenderer::GetDepthReadType();
	return desc;
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
		if (lights[i]->GetType() == ELightType::Point)
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
		else if (lights[i]->GetType() == ELightType::Directional)
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
		for (int i = 0; i < RHI::GetMGPUSettings()->MAX_PRESAMPLED_SHADOWS; i++)
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
	//Report Stats;
	std::string report = "Shadow Report: ";

	int Lights1 = 0;
	int lights0 = 0;
	for (int i = 0; i < ShadowingPointLights.size(); i++)
	{
		if (ShadowingPointLights[i]->GPUShadowResidentMask[0])
		{
			lights0++;
		}
		if (ShadowingPointLights[i]->GPUShadowResidentMask[1])
		{
			Lights1++;
		}
	}
	report += " GPU 0: " + std::to_string(lights0) + " GPU1: " + std::to_string(Lights1);
	Log::LogMessage(report);

	for (int spli = 0; spli < ShadowingPointLights.size(); spli++)
	{
		LightInteractions.push_back(new ShadowLightInteraction(ShadowingPointLights[spli]));
	}

	for (int i = 0; i < MAX_GPU_DEVICE_COUNT; i++)
	{
		if (i >= RHI::GetDeviceCount())
		{
			continue;
		}

		for (int x = 0; x < RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS; x++)
		{
			DSOs[i].ShadowCubeArray->SetIndexNull(x);
		}
		int slots = 0;
		for (int x = 0; x < LightInteractions.size(); x++)
		{
			if (LightInteractions[x]->IsResident(RHI::GetDeviceContext(i)))
			{
				DSOs[i].ShadowCubeArray->AddFrameBufferBind(LightInteractions[x]->GetMap(i), slots);
				slots++;
			}
		}
		RHIPipeLineStateDesc desc;
		desc.InitOLD(true, false, false);
		desc.RenderTargetDesc = GetCubeMapDesc();
		desc.ShaderInUse = PointLightShader;
		if (RHI::GetRenderSettings()->UseViewInstancing)
		{
			desc.ViewInstancing.Active = true;
			//dx12 max count is 4 for some reason so two batches of 3 instances
			desc.ViewInstancing.Instances = ViewInstancesPerDraw;
		}
		DSOs[i].PointLightShadowList->SetPipelineStateDesc(desc);
		desc.ShaderInUse = DirectionalLightShader;
		desc.FrameBufferTarget = DirectionalLightBuffer;
		DSOs[i].DirectionalShadowList->SetPipelineStateDesc(desc);
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

ShadowLightInteraction::ShadowLightInteraction(Light* light)
{
	lightPtr = light;
	for (int i = 0; i < RHI::GetDeviceCount(); i++)
	{
		if (lightPtr->GPUShadowResidentMask[i])
		{
			CreateRenderTargets(RHI::GetDeviceContext(i));
		}
		if (lightPtr->GPUShadowCopyDeviceTarget[i] != -1)
		{
			ShadowRenderer::GetSampleController(lightPtr->GPUShadowCopyDeviceTarget[i])->AddLight(this);
		}
	}
	//DeviceIndex = ResidentDevContext->GetDeviceIndex();
}

void ShadowLightInteraction::CreateRenderTargets(DeviceContext* context)
{
	const int index = context->GetDeviceIndex();
	int size = lightPtr->Resolution;
	if (lightPtr->GetShadowMode() == EShadowCaptureType::Baked)
	{
		size = lightPtr->BakedResolution;
	}
	if (lightPtr->GetType() == ELightType::Point)
	{
		ShadowMap[index] = RHI::CreateFrameBuffer(ResidentDevContext, ShadowRenderer::GetCubeMapFBDesc(size));
	}
	if (lightPtr->GetShadowMode() == EShadowCaptureType::Baked)
	{
		StaticShadowMap[index] = ShadowMap[index];
	}
	Shader = new Shader_Depth(ResidentDevContext, IsPointLight);
}

ShadowLightInteraction::~ShadowLightInteraction()
{
	SafeDelete(Shader);
	EnqueueSafeArrayRelease(ShadowMap, MAX_GPU_DEVICE_COUNT);
	EnqueueSafeArrayRelease(StaticShadowMap, MAX_GPU_DEVICE_COUNT);
}

void ShadowLightInteraction::SetupCopy(DeviceContext * TargetDev)
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
	desc.RTFormats[0] = ShadowRenderer::GetPreSampledTextureFormat(dev->GetDeviceIndex());
	desc.LinkToBackBufferScaleFactor = RHI::GetMGPUSettings()->PreSampleBufferScale;
	DSOs[dev->GetDeviceIndex()].PreSampledBuffer = RHI::CreateFrameBuffer(dev, desc);
	RHI::AddLinkedFrameBuffer(DSOs[dev->GetDeviceIndex()].PreSampledBuffer);
}

eTEXTURE_FORMAT ShadowRenderer::GetPreSampledTextureFormat(int deviceindex)
{
	int MaxShadows = 0;
	if (deviceindex == 0)
	{
		MaxShadows = RHI::GetMGPUSettings()->MAX_PRESAMPLED_SHADOWS_GPU0;
	}
	else
	{
		MaxShadows = RHI::GetMGPUSettings()->MAX_PRESAMPLED_SHADOWS;
	}
	Log::LogMessage("GPU" + std::to_string(deviceindex) + " Created shadow buffer of size " + std::to_string(MaxShadows));
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

FrameBuffer * ShadowLightInteraction::GetMap(int index)
{
	if (lightPtr->GetShadowMode() == EShadowCaptureType::Baked)
	{
		return StaticShadowMap[index];
	}
	return ShadowMap[index];
}

void ShadowLightInteraction::CopyDepth()
{}

bool ShadowLightInteraction::IsResident(DeviceContext * dev) const
{
	return lightPtr->GPUShadowResidentMask[dev->GetDeviceIndex()];
}

bool ShadowLightInteraction::NeedsRenderOnDevice(DeviceContext * dev)
{
	if (SampleOnAllDevices)
	{
		return true;
	}
	if (dev->GetDeviceIndex() != DeviceIndex)
	{
		return false;
	}
	return true;
}

bool ShadowLightInteraction::NeedsPresample(DeviceContext * dev)
{
	if (lightPtr->GPUShadowResidentMask[dev->GetDeviceIndex()])
	{
		if (lightPtr->GPUShadowCopyDeviceTarget[dev->GetDeviceIndex()] != dev->GetDeviceIndex())
		{
			return true;
		}
	}
	return false;
}

bool ShadowLightInteraction::NeedsUpdate()
{
	if (lightPtr->GetShadowMode() == EShadowCaptureType::Baked)
	{
		return !captured;
	}
	return true;
}

void ShadowLightInteraction::Invalidate()
{
	captured = false;
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

void ShadowRenderer::InvalidateAllBakedShadows()
{
	for (int i = 0; i < LightInteractions.size(); i++)
	{
		LightInteractions[i]->Invalidate();
	}
}

void PreSampleController::AddLight(ShadowLightInteraction * light)
{}
