#include "ShadowRenderer.h"
#include "RHI/RHI.h"
#include "Rendering/Shaders/Shader_Main.h"
#include "Rendering/Core/FrameBuffer.h"
#include "Core/Utils/MemoryUtils.h"
#include "Light.h"
#include "Rendering\Shaders\Shader_Depth.h"
#include "Core/GameObject.h"
#include "../RHI/RenderAPIs/D3D12/D3D12TimeManager.h"
#include "../RHI/DeviceContext.h"
#define CUBE_SIDES 6
ShadowRenderer::ShadowRenderer()
{
	DirectionalLightShader = new Shader_Depth(false);
	PointLightShader = new Shader_Depth(true);
	int shadowwidth = 512;
	ShadowDirectionalArray = RHI::CreateTextureArray(RHI::GetDeviceContext(0), MAX_DIRECTIONAL_SHADOWS);
	for (int i = 0; i < MAX_DIRECTIONAL_SHADOWS; i++)
	{
		DirectionalLightBuffers.push_back(RHI::CreateFrameBuffer(shadowwidth, shadowwidth, nullptr, 1, FrameBuffer::Depth));
		ShadowDirectionalArray->AddFrameBufferBind(DirectionalLightBuffers[i], i);
	}
	ShadowCubeArray = RHI::CreateTextureArray(RHI::GetDeviceContext(0), MAX_POINT_SHADOWS);
	for (int i = 0; i < MAX_POINT_SHADOWS; i++)
	{
		PointLightBuffers.push_back(RHI::CreateFrameBuffer(shadowwidth, shadowwidth, nullptr, 1, FrameBuffer::CubeDepth));
		ShadowCubeArray->AddFrameBufferBind(PointLightBuffers[i], i);
	}
	ShadowCubeArray->SetIndexNull(2);
	GeometryProjections = RHI::CreateRHIBuffer(RHIBuffer::Constant, nullptr);
	GeometryProjections->CreateConstantBuffer(sizeof(glm::mat4) * CUBE_SIDES, MAX_POINT_SHADOWS);
	PointShadowList = RHI::CreateCommandList();
	DirectionalShadowList = RHI::CreateCommandList();
}

ShadowRenderer::~ShadowRenderer()
{
	delete GeometryProjections;
	delete PointLightShader;
	delete DirectionalLightShader;
	MemoryUtils::DeleteVector(DirectionalLightBuffers);
	MemoryUtils::DeleteVector(PointLightBuffers);
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
		PointShadowList->ResetList();
		
		PointShadowList->GetDevice()->GetTimeManager()->StartTotalGPUTimer(PointShadowList);

		PointShadowList->GetDevice()->GetTimeManager()->StartTimer(PointShadowList, D3D12TimeManager::eGPUTIMERS::PointShadows);
		RenderPointShadows(PointShadowList, mainshader, ShadowObjects);
		PointShadowList->GetDevice()->GetTimeManager()->EndTimer(PointShadowList, D3D12TimeManager::eGPUTIMERS::PointShadows);
		PointShadowList->Execute();
	}
}
void ShadowRenderer::RenderPointShadows(RHICommandList * list, Shader_Main * mainshader, const std::vector<GameObject *> & ShadowObjects)
{
	for (int SNum = 0; SNum < (int)ShadowingPointLights.size(); SNum++)
	{
		FrameBuffer* TargetBuffer = PointLightBuffers[SNum];
		list->SetRenderTarget(TargetBuffer);
		list->ClearFrameBuffer(TargetBuffer);
		UpdateGeometryShaderParams(ShadowingPointLights[SNum]->GetPosition(), ShadowingPointLights[SNum]->Projection, SNum);
		list->SetConstantBufferView(GeometryProjections, SNum, 0);
		Shader_Depth::LightData data = {};
		data.Proj = ShadowingPointLights[SNum]->Projection;
		data.Lightpos = ShadowingPointLights[SNum]->GetPosition();
		PointLightShader->UpdateBuffer(list, &data, SNum);
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
	//return;
	ShadowDirectionalArray->BindToShader(list, 4);	
	ShadowCubeArray->BindToShader(list, 5);
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
				std::cout << "Max point Shadows Hit" << std::endl;
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
				std::cout << "Max Directional Shadows Hit" << std::endl;
				lights[i]->SetShadow(false);
			}
		}
	}
	PointShadowList->SetPipelineState(PipeLineState{ true,false,false });
	PointShadowList->CreatePipelineState(PointLightShader, PointLightBuffers[0]);//all show buffers for cube maps are the same!
	DirectionalShadowList->SetPipelineState(PipeLineState{ true,false,false });
	DirectionalShadowList->CreatePipelineState(DirectionalLightShader, DirectionalLightBuffer);
	RHI::CreateFrameBuffer(RHI::GetDeviceContext(0), RHIFrameBufferDesc::CreateColourDepth(1024,1024));
	RHI::CreateFrameBuffer(RHI::GetDeviceContext(0), RHIFrameBufferDesc::CreateColour(1024, 1024));
	RHI::CreateFrameBuffer(RHI::GetDeviceContext(0), RHIFrameBufferDesc::CreateDepth(1024, 1024));
	RHI::CreateFrameBuffer(RHI::GetDeviceContext(0), RHIFrameBufferDesc::CreateCubeDepth(1024, 1024));
}
