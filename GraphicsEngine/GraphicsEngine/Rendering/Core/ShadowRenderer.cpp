#include "ShadowRenderer.h"
#include "RHI/RHI.h"
#include "Rendering/Shaders/Shader_Main.h"
#include "Rendering/Core/FrameBuffer.h"
#include "Core/Utils/MemoryUtils.h"
#include "Light.h"
#include "Rendering\Shaders\Shader_Depth.h"
#include "../Rendering/Shaders/MultiGPU/Shader_ShadowSample.h"
#include "Core/GameObject.h"
#include "RHI/RenderAPIs/D3D12/D3D12TimeManager.h"
#include "RHI/RenderAPIs/D3D12/D3D12Framebuffer.h"
#include "RHI/RenderAPIs/D3D12/D3D12CommandList.h"
#include "RHI/DeviceContext.h"
#define CUBE_SIDES 6

ShadowRenderer::ShadowRenderer()
{
	DirectionalLightShader = new Shader_Depth(false);

	int shadowwidth = 1024;
	ShadowDirectionalArray = RHI::CreateTextureArray(RHI::GetDeviceContext(0), MAX_DIRECTIONAL_SHADOWS);
	for (int i = 0; i < MAX_DIRECTIONAL_SHADOWS; i++)
	{
		DirectionalLightBuffers.push_back(RHI::CreateFrameBuffer(RHI::GetDeviceContext(0), RHIFrameBufferDesc::CreateDepth(shadowwidth, shadowwidth)));
		ShadowDirectionalArray->AddFrameBufferBind(DirectionalLightBuffers[i], i);
	}
	ShadowCubeArray = RHI::CreateTextureArray(RHI::GetDeviceContext(0), MAX_POINT_SHADOWS);
#if 0
	for (int i = 0; i < MAX_POINT_SHADOWS; i++)
	{
		PointLightBuffers.push_back(RHI::CreateFrameBuffer(RHI::GetDeviceContext(0), RHIFrameBufferDesc::CreateCubeDepth(shadowwidth, shadowwidth)));
		ShadowCubeArray->AddFrameBufferBind(PointLightBuffers[i], i);
	}
#else
	for (int i = 0; i < MAX_POINT_SHADOWS; i++)
	{
		if (i == 2 && RHI::GetMGPUMode()->SplitShadowWork)
		{
			LightInteractions.push_back(new ShadowLightInteraction(RHI::GetDeviceContext(1), true));
			ShadowCubeArray->SetIndexNull(2);
		}
		else
		{
			LightInteractions.push_back(new ShadowLightInteraction(RHI::GetDeviceContext(0), true));
			ShadowCubeArray->AddFrameBufferBind(LightInteractions[i]->ShadowMap, i);
		}
	}
#endif
	DeviceContext* pointlightdevice = RHI::GetDeviceContext(0);
	//ShadowCubeArray->SetIndexNull(2);
	if (MAX_POINT_SHADOWS > 0)
	{
		PointLightShader = new Shader_Depth(true, pointlightdevice);
		GeometryProjections = RHI::CreateRHIBuffer(RHIBuffer::Constant, pointlightdevice);
		GeometryProjections->CreateConstantBuffer(sizeof(glm::mat4) * CUBE_SIDES, MAX_POINT_SHADOWS, true);
		PointShadowList = RHI::CreateCommandList(ECommandListType::Graphics, pointlightdevice);
		PointShadowListALT = RHI::CreateCommandList(ECommandListType::Graphics, RHI::GetDeviceContext(1));
	}
	DirectionalShadowList = RHI::CreateCommandList();
	ShadowPreSampleShader = new Shader_ShadowSample(RHI::GetDeviceContext(1));
	ShadowPreSamplingList = RHI::CreateCommandList(ECommandListType::Graphics, RHI::GetDeviceContext(1));
	ShadowPreSamplingList->CreatePipelineState(ShadowPreSampleShader, LightInteractions[2]->PreSampledBuffer);
}

ShadowRenderer::~ShadowRenderer()
{
	delete GeometryProjections;
	delete PointLightShader;
	delete DirectionalLightShader;
	MemoryUtils::DeleteVector(DirectionalLightBuffers);
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
		DirectionalShadowList->GetDevice()->GetTimeManager()->StartTimer(DirectionalShadowList, D3D12TimeManager::eGPUTIMERS::DirShadows);
		RenderDirectionalShadows(DirectionalShadowList, mainshader, ShadowObjects);
		DirectionalShadowList->GetDevice()->GetTimeManager()->EndTimer(DirectionalShadowList, D3D12TimeManager::eGPUTIMERS::DirShadows);
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
	List->GetDevice()->GetTimeManager()->StartTimer(List, D3D12TimeManager::eGPUTIMERS::PointShadows);
	RenderPointShadows(List, mainshader, ShadowObjects);
	List->GetDevice()->GetTimeManager()->EndTimer(List, D3D12TimeManager::eGPUTIMERS::PointShadows);
	List->Execute();
}

void ShadowRenderer::PreSampleShadows(const std::vector<GameObject*>& ShadowObjects, Shader_Main* mainshader)
{
	RHICommandList* list = ShadowPreSamplingList;
	list->ResetList();
	list->GetDevice()->GetTimeManager()->StartTimer(list, D3D12TimeManager::eGPUTIMERS::ShadowPreSample);
	for (int SNum = 0; SNum < (int)ShadowingPointLights.size(); SNum++)
	{
		if (!LightInteractions[SNum]->NeedsSample)
		{
			continue;
		}
		list->SetFrameBufferTexture(LightInteractions[SNum]->ShadowMap, 0);
		list->SetRenderTarget(LightInteractions[SNum]->PreSampledBuffer);
		list->ClearFrameBuffer(LightInteractions[SNum]->PreSampledBuffer);
		mainshader->BindLightsBuffer(list);
		mainshader->BindMvBuffer(list, 2);
		for (size_t i = 0; i < ShadowObjects.size(); i++)
		{
			if (ShadowObjects[i]->GetMat() == nullptr)
			{
				//object should not be rendered to the depth map
				continue;
			}
			if (ShadowObjects[i]->GetMat()->GetDoesShadow() == false)
			{
				continue;
			}
			mainshader->SetActiveIndex(list, (int)i, list->GetDeviceIndex());
			ShadowObjects[i]->Render(true, list);
		}
		list->SetRenderTarget(nullptr);
	}
	list->GetDevice()->GetTimeManager()->EndTimer(list, D3D12TimeManager::eGPUTIMERS::ShadowPreSample);
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
		UpdateGeometryShaderParams(ShadowingPointLights[SNum]->GetPosition(), ShadowingPointLights[SNum]->Projection, SNum);
		list->SetConstantBufferView(GeometryProjections, SNum, 0);
		Shader_Depth::LightData data = {};
		data.Proj = ShadowingPointLights[SNum]->Projection;
		data.Lightpos = ShadowingPointLights[SNum]->GetPosition();
		TargetShader->UpdateBuffer(list, &data, SNum);
		for (size_t i = 0; i < ShadowObjects.size(); i++)
		{
			if (ShadowObjects[i]->GetMat() == nullptr)
			{
				//object should not be rendered to the depth map
				continue;
			}
			if (ShadowObjects[i]->GetMat()->GetDoesShadow() == false)
			{
				continue;
			}
			mainshader->SetActiveIndex(list, (int)i, list->GetDeviceIndex());
			ShadowObjects[i]->Render(true, list);
		}
	}
}

void ShadowRenderer::RenderDirectionalShadows(RHICommandList * list, Shader_Main * mainshader, const std::vector<GameObject *> & ShadowObjects)
{
	for (size_t SNum = 0; SNum < ShadowingDirectionalLights.size(); SNum++)
	{
		FrameBuffer* TargetBuffer = DirectionalLightBuffers[SNum];

		list->SetRenderTarget(TargetBuffer);
		list->ClearFrameBuffer(TargetBuffer);
		mainshader->UpdateMV(ShadowingDirectionalLights[SNum]->DirView, ShadowingDirectionalLights[SNum]->Projection);
		Shader_Depth::LightData data = {};
		data.Proj = ShadowingDirectionalLights[SNum]->Projection;
		data.Lightpos = ShadowingDirectionalLights[SNum]->GetPosition();
		DirectionalLightShader->UpdateBuffer(list, &data, SNum);
		for (size_t i = 0; i < ShadowObjects.size(); i++)
		{
			if (ShadowObjects[i]->GetMat() == nullptr)
			{
				//object should not be rendered to the depth map
				continue;
			}
			if (ShadowObjects[i]->GetMat()->GetDoesShadow() == false)
			{
				continue;
			}
			mainshader->SetActiveIndex(list, (int)i);
			ShadowObjects[i]->Render(true, list);
		}
		//list->SetRenderTarget(nullptr);
	}
}

void ShadowRenderer::BindShadowMapsToTextures(RHICommandList * list)
{
	ShadowDirectionalArray->BindToShader(list, 4);
	ShadowCubeArray->BindToShader(list, 5);
#if 0
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

	int lastpointshadow = 0;
	int lastdirshadow = 0;
	for (size_t i = 0; i < lights.size(); i++)
	{
		if (lights[i]->GetDoesShadow() == false)
		{
			continue;
		}
		if (lights[i]->GetType() == Light::Point)
		{
			if (ShadowingPointLights.size() < MAX_POINT_SHADOWS)
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
			if (ShadowingDirectionalLights.size() < MAX_DIRECTIONAL_SHADOWS)
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
	PointShadowList->SetPipelineState(PipeLineState{ true,false,false });
	PointShadowList->CreatePipelineState(PointLightShader, LightInteractions[0]->ShadowMap);//all show buffers for cube maps are the same!

	PointShadowListALT->SetPipelineState(PipeLineState{ true,false,false });
	PointShadowListALT->CreatePipelineState(PointLightShader, LightInteractions[0]->ShadowMap);//all show buffers for cube maps are the same!

	DirectionalShadowList->SetPipelineState(PipeLineState{ true,false,false });
	DirectionalShadowList->CreatePipelineState(DirectionalLightShader, DirectionalLightBuffer);

}
#include "../RHI/RenderAPIs/D3D12/D3D12Framebuffer.h"
#include "../RHI/RenderAPIs/D3D12/D3D12CommandList.h"
#include "../RHI/RenderAPIs/D3D12/GPUResource.h"
void ShadowRenderer::Unbind(RHICommandList * list)
{
	if (RHI::GetMGPUMode()->SplitShadowWork)
	{
		D3D12FrameBuffer* dBuffer = (D3D12FrameBuffer*)LightInteractions[2]->PreSampledBuffer;
		dBuffer->MakeReadyForCopy(((D3D12CommandList*)list)->GetCommandList());
	}
}

ShadowRenderer::ShadowLightInteraction::ShadowLightInteraction(DeviceContext * Context, bool IsPoint)
{
	DeviceIndex = Context->GetDeviceIndex();
	if (IsPoint)
	{
		int size = (Context->GetDeviceIndex() == 0) ? ShadowSize : ShadowSize/1.2;
		ShadowMap = RHI::CreateFrameBuffer(Context, RHIFrameBufferDesc::CreateCubeDepth(size, size));
	}
	Shader = new Shader_Depth(IsPoint, Context);
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

void ShadowRenderer::ShadowLightInteraction::PreSampleShadows(RHICommandList * List)
{

}
