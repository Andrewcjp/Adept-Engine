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
#include "Light.h"
#include "ShadowAtlas.h"
#include "LightCulling/LightCullingEngine.h"
#include "Core/BaseWindow.h"
#include "Core/Assets/Scene.h"
#define SINGLE_GPU_PRESAMPLE 0
#define CUBE_SIDES 6
#define TEST_PRESAMPLE 1

ShadowRenderer* ShadowRenderer::Instance = nullptr;

ShadowRenderer::ShadowRenderer()
{
	Instance = this;
}

ShadowRenderer::~ShadowRenderer()
{}

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

//void ShadowRenderer::PreSampleShadows(RHICommandList* list)
//{
//	SCOPE_CYCLE_COUNTER("PreSampleShadows");
//	if (!NeedsCopyPreSample[list->GetDeviceIndex()])
//	{
//		return;
//	}
//	list->ResetList();
//	/*if (RHI::GetMGPUSettings()->AsyncShadows && list->GetDeviceIndex() > 0)
//	{
//		list->GetDevice()->GetTimeManager()->StartTotalGPUTimer(list);
//	}*/
//	list->StartTimer(EGPUTIMERS::ShadowPreSample);
//	//ensure(SampledLightInteractions < RHI::GetMGPUMode()->MAX_PRESAMPLED_SHADOWS);
//	const int DeviceIndex = list->GetDeviceIndex();
//	int MaxShadow = RHI::GetMGPUSettings()->MAX_PRESAMPLED_SHADOWS;
//	if (DeviceIndex == 0)
//	{
//		MaxShadow = RHI::GetMGPUSettings()->MAX_PRESAMPLED_SHADOWS_GPU0;
//	}
//	int ShaderData[4] = { 0 };
//	int CurrnetIndex = 0;
//	for (int i = 0; i < LightInteractions.size(); i++)
//	{
//		if (LightInteractions[i]->DeviceIndex != DeviceIndex)
//		{
//			continue;
//		}
//		if (!LightInteractions[i]->NeedsSample)
//		{
//			continue;
//		}
//		if (CurrnetIndex >= MaxShadow)
//		{
//			Log::LogMessage("Too many PreSampled Shadows Lights", Log::Severity::Error);
//			break;
//		}
//		int id = -1;
//		for (int x = 0; x < ShadowingPointLights.size(); x++)
//		{
//			if (ShadowingPointLights[x] == LightInteractions[i]->lightPtr)
//			{
//				id = x;
//				break;
//			}
//		}
//		if (id == -1)
//		{
//			continue;
//		}
//		ShaderData[CurrnetIndex] = id;
//		CurrnetIndex++;
//	}
//	list->SetRootConstant(Shader_ShadowSample::PreSampleCBV, 4, ShaderData, 0);
//	DSOs[DeviceIndex].ShadowCubeArray->BindToShader(list, Shader_ShadowSample::ShadowSRV);
//	//list->SetRenderTarget(DSOs[DeviceIndex].PreSampledBuffer);
//	//list->ClearFrameBuffer(DSOs[DeviceIndex].PreSampledBuffer);
//	list->BeginRenderPass(RHIRenderPassDesc(DSOs[DeviceIndex].PreSampledBuffer, ERenderPassLoadOp::Clear));
//	Scenerenderer->BindMvBuffer(list, Shader_Depth_RSSlots::VPBuffer);
//	Scenerenderer->BindLightsBuffer(list, 1);
//	Scenerenderer->MeshController->RenderPass(ERenderPass::DepthOnly, list, nullptr);
//	list->EndRenderPass();
//
//	list->EndTimer(EGPUTIMERS::ShadowPreSample);
//	/*if (RHI::GetMGPUSettings()->SplitShadowWork && !RHI::GetMGPUSettings()->MainPassSFR && list->GetDeviceIndex() != 0 && !RHI::GetMGPUSettings()->AsyncShadows)
//	{
//		list->GetDevice()->GetTimeManager()->EndTotalGPUTimer(list);
//	}*/
//	if (list->GetDeviceIndex() == 1)
//	{
//		list->GetDevice()->InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::InterCopy);
//	}
//	list->Execute();
//
//#if !SINGLE_GPU_PRESAMPLE
//	if (DSOs[list->GetDeviceIndex()].PreSampledBuffer != nullptr)
//	{
//		//if (!RHI::GetMGPUSettings()->AsyncShadows)
//		{
//			if (list->GetDeviceIndex() == 0)
//			{
//				if (NeedsCopyPreSample[0])
//				{
//					//SFR shadows is delaying all graphics work unitl copy complete point
//					//FrameBuffer::CopyHelper(DSOs[list->GetDeviceIndex()].PreSampledBuffer, RHI::GetDeviceContext(1), EGPUCOPYTIMERS::ShadowCopy2);
//					FrameBuffer::CopyHelper_NewSync(DSOs[list->GetDeviceIndex()].PreSampledBuffer, RHI::GetDeviceContext(1), EGPUCOPYTIMERS::ShadowCopy2);
//				}
//			}
//			else
//			{
//				if (NeedsCopyPreSample[1])
//				{
//					FrameBuffer::CopyHelper(DSOs[list->GetDeviceIndex()].PreSampledBuffer, RHI::GetDeviceContext(0), EGPUCOPYTIMERS::ShadowCopy);
//				}
//			}
//		}
//	}
//#endif
//
//}

void ShadowRenderer::RenderPointShadows(RHICommandList * list)
{
	list->StartTimer(EGPUTIMERS::PointShadows);
	int IndexOnGPU = 0;
	std::vector<Light*> lights = SceneRenderer::Get()->GetLightCullingEngine()->GetCurrentlyRelevantLights();
	for (int i = 0; i < lights.size(); i++)
	{
		Light* Ptr = lights[i];
		if (!Ptr->GetDoesShadow())
		{
			continue;
		}
		if (!Ptr->IsResident(list->GetDevice()))
		{
			continue;
		}
		if (!Ptr->NeedsShadowUpdate())
		{
			continue;
		}
		if (!Ptr->HasValidHandle(list->GetDeviceIndex()))
		{
			Log::LogMessage("Light has invalid handle", Log::Error);
			continue;
		}
		if (RHI::GetRenderSettings()->GetShadowSettings().UseGeometryShaderForShadows)
		{
			RenderShadowMap_GPU(lights[i], list, IndexOnGPU);
		}
		else
		{
			RenderShadowMap_CPU(lights[i], list, IndexOnGPU);
		}
		Ptr->NotifyShadowUpdate();
		IndexOnGPU++;
	}
	/*if (list->GetDeviceIndex() == 0 && RHI::GetMGPUSettings()->SplitShadowWork)
	{
		list->InsertGPUStallTimer();
	}*/
	list->EndTimer(EGPUTIMERS::PointShadows);
}

void ShadowRenderer::RenderShadowMap_GPU(Light* LightPtr, RHICommandList * list, int IndexOnGPU)
{
	//return;
	SetPointRS(list);
	SceneRenderer::Get()->GetCullingManager()->UpdateCullingForShadowLight(LightPtr, SceneRenderer::Get()->GetScene());

	FrameBuffer* TargetBuffer = LightPtr->GPUResidenceMask[list->GetDeviceIndex()].AtlasHandle->DynamicMapPtr;
	Shader_Depth* TargetShader = ShaderComplier::GetShader<Shader_Depth>(list->GetDevice(), true);

	list->BeginRenderPass(RHIRenderPassDesc(TargetBuffer, ERenderPassLoadOp::Clear));
	TargetShader->UpdateGeometryShaderParams(LightPtr->GetPosition(), LightPtr->Projection, IndexOnGPU);
	TargetShader->SetProjections(list, IndexOnGPU);

	Shader_Depth::LightData data = {};
	data.Proj = LightPtr->Projection;
	data.Lightpos = LightPtr->GetPosition();
	TargetShader->UpdateBuffer(list, &data, IndexOnGPU);
	list->SetSingleRootConstant(Shader_Depth_RSSlots::VI_Offset, 0);
	EBatchFilter::Type Filter = EBatchFilter::ALL;
	if (LightPtr->GetLightMobility() == ELightMobility::Baked || LightPtr->GetLightMobility() == ELightMobility::Stationary)
	{
		Filter = EBatchFilter::StaticOnly;
	}
	if (RHI::GetRenderSettings()->GetShadowSettings().UseViewInstancingForShadows)
	{
		for (int i = 0; i < CUBE_SIDES; i += RHI::GetRenderSettings()->GetShadowSettings().ViewInstancesPerDraw)
		{
			list->SetSingleRootConstant(Shader_Depth_RSSlots::VI_Offset, i);
			SceneRenderer::Get()->MeshController->RenderPass(ERenderPass::DepthOnly, list, TargetShader, Filter);
		}
	}
	else
	{
		SceneRenderer::Get()->MeshController->RenderPass(ERenderPass::DepthOnly, list, TargetShader, Filter);
	}
	list->EndRenderPass();
	TargetBuffer->MakeReadyForComputeUse(list, true);
}

void ShadowRenderer::RenderShadowMap_CPU(Light* LightPtr, RHICommandList * list, int IndexOnGPU)
{
	SceneRenderer::Get()->GetCullingManager()->UpdateCullingForShadowLight(LightPtr, SceneRenderer::Get()->GetScene());
	FrameBuffer* TargetBuffer = LightPtr->GPUResidenceMask[list->GetDeviceIndex()].AtlasHandle->DynamicMapPtr;
	Shader_Depth* TargetShader = ShaderComplier::GetShader<Shader_Depth>(list->GetDevice());

	list->BeginRenderPass(RHIRenderPassDesc(TargetBuffer, ERenderPassLoadOp::Clear));
	TargetShader->UpdateGeometryShaderParams(LightPtr->GetPosition(), LightPtr->Projection, IndexOnGPU);
	TargetShader->SetProjections(list, IndexOnGPU);

	Shader_Depth::LightData data = {};
	data.Proj = LightPtr->Projection;
	data.Lightpos = LightPtr->GetPosition();
	TargetShader->UpdateBuffer(list, &data, IndexOnGPU);
	for (int Faces = 0; Faces < 6; Faces++)
	{
		list->SetSingleRootConstant(Shader_Depth_RSSlots::VI_Offset, Faces);
		SceneRenderer::Get()->MeshController->RenderPass(ERenderPass::DepthOnly, list, TargetShader);
	}
	list->EndRenderPass();
}

void ShadowRenderer::RenderDirectionalShadows(RHICommandList * list)
{
	/*for (int SNum = 0; SNum < (int)LightInteractions.size(); SNum++)
	{
		if (!LightInteractions[SNum]->lightPtr->GetType() == ELightType::Directional)
		{
			continue;
		}
		FrameBuffer* TargetBuffer = LightInteractions[SNum]->GetMap(list->GetDeviceIndex());
		list->BeginRenderPass(RHIRenderPassDesc(TargetBuffer, ERenderPassLoadOp::Clear));
		Shader_Depth::LightData data = {};
		data.Proj = ShadowingDirectionalLights[SNum]->Projection;
		data.Lightpos = ShadowingDirectionalLights[SNum]->GetPosition();
		DirectionalLightShader->UpdateBuffer(list, &data, (int)SNum);
		Scenerenderer->MeshController->RenderPass(ERenderPass::DepthOnly, list, LightInteractions[SNum]->Shader);
		list->EndRenderPass();
	}*/
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

void ShadowRenderer::SetPointRS(RHICommandList* list)
{
	RHIPipeLineStateDesc desc;
	desc.InitOLD(true, false, false);
	desc.RenderTargetDesc = GetCubeMapDesc();
	desc.ShaderInUse = ShaderComplier::GetShader<Shader_Depth>(list->GetDevice(), true);
	if (RHI::GetRenderSettings()->GetShadowSettings().UseViewInstancingForShadows)
	{
		desc.ViewInstancing.Active = true;
		//dx12 max count is 4 for some reason so two batches of 3 instances
		desc.ViewInstancing.Instances = RHI::GetRenderSettings()->GetShadowSettings().ViewInstancesPerDraw;
	}
	list->SetPipelineStateDesc(desc);
}

//void ShadowRenderer::InitPreSampled(DeviceContext* dev, DeviceContext* Targetdev)
//{
//	if (Targetdev == nullptr)
//	{
//		return;
//	}
//	if (DSOs[dev->GetDeviceIndex()].PreSampledBuffer != nullptr)
//	{
//		RHI::RemoveLinkedFrameBuffer(DSOs[dev->GetDeviceIndex()].PreSampledBuffer);
//		EnqueueSafeRHIRelease(DSOs[dev->GetDeviceIndex()].PreSampledBuffer);
//	}
//	const int size = 512;
//	RHIFrameBufferDesc desc = RHIFrameBufferDesc::CreateColourDepth(size, size);
//	desc.IsShared = true;
//	desc.DeviceToCopyTo = Targetdev;
//	desc.RTFormats[0] = ShadowRenderer::GetPreSampledTextureFormat(dev->GetDeviceIndex());
//	desc.LinkToBackBufferScaleFactor = RHI::GetMGPUSettings()->PreSampleBufferScale;
//	DSOs[dev->GetDeviceIndex()].PreSampledBuffer = RHI::CreateFrameBuffer(dev, desc);
//	RHI::AddLinkedFrameBuffer(DSOs[dev->GetDeviceIndex()].PreSampledBuffer);
//}

eTEXTURE_FORMAT ShadowRenderer::GetPreSampledTextureFormat(int Shadownumber)
{
	if (Shadownumber == 1)
	{
		return eTEXTURE_FORMAT::FORMAT_R8_UNORM;
	}
	else if (Shadownumber == 2)
	{
		return eTEXTURE_FORMAT::FORMAT_R8G8_UNORM;
	}
	//todo: don't copy alpha channel of 3 lights !
	return eTEXTURE_FORMAT::FORMAT_R8G8B8A8_UNORM;
}

void ShadowRenderer::InvalidateAllBakedShadows()
{
	std::vector<Light*> lights = BaseWindow::GetScene()->GetLights();
	for (int i = 0; i < lights.size(); i++)
	{
		lights[i]->InvalidateCachedShadow();
	}
}

void ShadowRenderer::AssignAtlasData(ShadowAtlas* Node)
{
	std::vector<Light*> lights = SceneRenderer::Get()->GetLightCullingEngine()->GetCurrentlyRelevantLights();
	//#Shadow: todo: MGPU
	int DeviceIndex = 0;
	for (int i = 0; i < lights.size(); i++)
	{
		if (!lights[i]->GetDoesShadow())
		{
			continue;
		}
		if (lights[i]->HasValidHandle(DeviceIndex))
		{
			continue;
		}
		if (!lights[i]->IsResident(RHI::GetDeviceContext(DeviceIndex)))
		{
			continue;
		}
		lights[i]->SetShadowId(i);
		lights[i]->GPUResidenceMask[DeviceIndex].AtlasHandle = Node->AllocateHandle(lights[i], RHI::GetDeviceContext(DeviceIndex));
	}

}

ShadowRenderer * ShadowRenderer::Get()
{
	return Instance;
}

void ShadowRenderer::UpdateShadowAsignments()
{
	for (int i = 0; i < AtlasSets.size(); i++)
	{
		AssignAtlasData(AtlasSets[i]);
	}
}
