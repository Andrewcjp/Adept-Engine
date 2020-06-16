#include "ShadowRenderer.h"
#include "Core/GameObject.h"
#include "Core/Performance/PerfManager.h"
#include "Rendering/Shaders/MultiGPU/Shader_ShadowSample.h"
#include "Rendering/Shaders/Shader_Deferred.h"
#include "Rendering/Shaders/Shadow/Shader_Depth.h"
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
#include "RHI/RHITimeManager.h"
#include "Rendering/RenderNodes/RenderNode.h"
#include "Rendering/RenderNodes/Nodes/ShadowUpdateNode.h"
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
ETextureFormat ShadowRenderer::GetDepthType()
{
#if USE32BITDepth
	return ETextureFormat::FORMAT_R32_TYPELESS;
#else
	return ETextureFormat::FORMAT_D16_UNORM;
#endif
}

ETextureFormat ShadowRenderer::GetDepthReadType()
{
#if USE32BITDepth
	return ETextureFormat::R32_FLOAT;
#else
	return ETextureFormat::FORMAT_R16_UNORM;
#endif
}

void ShadowRenderer::RenderPointShadows(RHICommandList* list)
{
	DECALRE_SCOPEDGPUCOUNTER(list, ShadowUpdateNode::GetNodeName()+std::string(".Point"));
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
		if (!Ptr->NeedsShadowUpdate(list->GetDeviceIndex()))
		{
			continue;
		}
		if (!Ptr->HasValidHandle(list->GetDeviceIndex()))
		{
			Log::LogMessage("Light has invalid handle", Log::Error);
			continue;
		}
		if (IndexOnGPU >= RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS)
		{
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
		Ptr->NotifyShadowUpdate(list->GetDeviceIndex());
		IndexOnGPU++;
	}
	/*if (list->GetDeviceIndex() == 0 && RHI::GetMGPUSettings()->SplitShadowWork)
	{
		list->InsertGPUStallTimer();
	}*/
	
}

void ShadowRenderer::RenderShadowMap_GPU(Light* LightPtr, RHICommandList* list, int IndexOnGPU)
{
	//return;

	SceneRenderer::Get()->GetCullingManager()->UpdateCullingForShadowLight(LightPtr, SceneRenderer::Get()->GetScene());

	FrameBuffer* TargetBuffer = LightPtr->GPUResidenceMask[list->GetDeviceIndex()].AtlasHandle->DynamicMapPtr;
	SetPointRS(list, TargetBuffer);
	Shader_Depth* TargetShader = ShaderCompiler::GetShader<Shader_Depth>(list->GetDevice(), true);
	RHIRenderPassDesc D = RHIRenderPassDesc(TargetBuffer, ERenderPassLoadOp::Clear);
	D.FinalState = EResourceState::PixelShader;
	if (RHI::GetFrameCount() != 0)
	{
		D.InitalState = EResourceState::Undefined;
	}
	list->BeginRenderPass(D);
	TargetShader->UpdateGeometryShaderParams(LightPtr->GetPosition(), LightPtr->Projection, IndexOnGPU);
	TargetShader->SetProjections(list, IndexOnGPU);

	Shader_Depth::LightData data = {};
	data.Proj = LightPtr->Projection;
	data.Lightpos = LightPtr->GetPosition();
	TargetShader->UpdateBuffer(list, &data, IndexOnGPU);
	if (RHI::GetRenderSettings()->GetShadowSettings().UseViewInstancingForShadows)
	{
		list->SetSingleRootConstant(Shader_Depth_RSSlots::VI_Offset, 0);
	}
	EBatchFilter::Type Filter = EBatchFilter::ALL;
	if (LightPtr->GetLightMobility() == ELightMobility::Baked || LightPtr->GetLightMobility() == ELightMobility::Stationary)
	{
		Filter = EBatchFilter::StaticOnly;
	}
	if (RHI::GetRenderSettings()->GetShadowSettings().UseViewInstancingForShadows)
	{
		for (int i = 0; i < CUBE_SIDES; i += RHI::GetRenderSettings()->GetShadowSettings().ViewInstancesPerDraw)
		{
			if (RHI::GetRenderSettings()->GetShadowSettings().UseViewInstancingForShadows)
			{
				list->SetSingleRootConstant(Shader_Depth_RSSlots::VI_Offset, i);
			}
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

void ShadowRenderer::RenderShadowMap_CPU(Light* LightPtr, RHICommandList* list, int IndexOnGPU)
{
	SceneRenderer::Get()->GetCullingManager()->UpdateCullingForShadowLight(LightPtr, SceneRenderer::Get()->GetScene());
	FrameBuffer* TargetBuffer = LightPtr->GPUResidenceMask[list->GetDeviceIndex()].AtlasHandle->DynamicMapPtr;
	Shader_Depth* TargetShader = ShaderCompiler::GetShader<Shader_Depth>(list->GetDevice());

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
	TargetBuffer->MakeReadyForComputeUse(list, true);
}

void ShadowRenderer::RenderDirectionalShadows(RHICommandList* list)
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

void ShadowRenderer::SetPointRS(RHICommandList* list, FrameBuffer* buffer)
{
	RHIPipeLineStateDesc desc;
	desc.InitOLD(true, false, false);
	desc.RenderTargetDesc = buffer->GetPiplineRenderDesc();
	desc.ShaderInUse = ShaderCompiler::GetShader<Shader_Depth>(list->GetDevice(), true);
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

ETextureFormat ShadowRenderer::GetPreSampledTextureFormat(int Shadownumber)
{
	if (Shadownumber == 1)
	{
		return ETextureFormat::FORMAT_R8_UNORM;
	}
	else if (Shadownumber == 2)
	{   
		return ETextureFormat::FORMAT_R8G8_UNORM;
	}
	//todo: don't copy alpha channel of 3 lights !
	return ETextureFormat::FORMAT_R8G8B8A8_UNORM;
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
	int DeviceIndex = Node->GetDeviceIndex();
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
		lights[i]->GPUResidenceMask[DeviceIndex].AtlasHandle->HandleId = i;
	}

}

//handles are in arrays of maps and need to be contiguous.
void ShadowRenderer::UpdateShadowID(Light* L, int D)
{
	if (L->GPUResidenceMask[D].AtlasHandle != nullptr)
	{
		L->SetShadowId(L->GPUResidenceMask[D].AtlasHandle->HandleId);
	}
}

ShadowRenderer* ShadowRenderer::Get()
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
