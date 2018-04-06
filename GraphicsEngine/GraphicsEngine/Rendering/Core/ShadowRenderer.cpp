#include "ShadowRenderer.h"
#include "RHI/RHI.h"
#include "../Rendering/Shaders/Shader_Main.h"
#include "../D3D12/D3D12RHI.h"
#include "../Rendering/Core/FrameBuffer.h"
#include "../Core/Utils/MemoryUtils.h"
ShadowRenderer::ShadowRenderer()
{
	DirectionalLightShader = new Shader_Depth(false);
	PointLightShader = new Shader_Depth(true);
	int shadowwidth = 1024;
	for (int i = 0; i < MAX_DIRECTIONAL_SHADOWS; i++)
	{
		DirectionalLightBuffers.push_back(RHI::CreateFrameBuffer(shadowwidth, shadowwidth, 1, FrameBuffer::Depth));
	}
	PointLightBuffer = RHI::CreateFrameBuffer(shadowwidth, shadowwidth, 1, FrameBuffer::CubeDepth);
	GeometryProjections = RHI::CreateRHIBuffer(RHIBuffer::Constant);
	GeometryProjections->CreateConstantBuffer(sizeof(glm::mat4x4), 6);
}

ShadowRenderer::~ShadowRenderer()
{
	delete PointLightBuffer;
	delete GeometryProjections;
	delete PointLightShader;
	delete DirectionalLightShader;
	MemoryUtils::DeleteVector(DirectionalLightBuffers);
}

void ShadowRenderer::UpdateGeometryShaderParams(glm::vec3 lightPos, glm::mat4 shadowProj)
{
	std::vector<glm::mat4> shadowTransforms;
	shadowTransforms.push_back(shadowProj *
		glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
	shadowTransforms.push_back(shadowProj *
		glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
	shadowTransforms.push_back(shadowProj *
		glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
	shadowTransforms.push_back(shadowProj *
		glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
	shadowTransforms.push_back(shadowProj *
		glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
	shadowTransforms.push_back(shadowProj *
		glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));
}

void ShadowRenderer::RenderShadowMaps(Camera * c, std::vector<Light*> lights, const std::vector<GameObject*>& ShadowObjects, RHICommandList* list, Shader_Main* mainshader)
{
	//if (UseCache)
	//{
	//	if (Renderered)
	//	{
	//		return;
	//	}
	//	else
	//	{
	//		Renderered = true;
	//	}
	//}
	//todo: refactor!

	//list->ResetList();
	RenderDirectionalShadows(list, mainshader, ShadowObjects);
	//RenderPointShadows(list, mainshader, ShadowObjects);

}
void ShadowRenderer::RenderPointShadows(RHICommandList * list, Shader_Main * mainshader, const std::vector<GameObject *> & ShadowObjects)
{
	for (size_t SNum = 0; SNum < ShadowingPointLights.size(); SNum++)
	{
		FrameBuffer* TargetBuffer = PointLightBuffer;

		list->SetRenderTarget(TargetBuffer);
		list->ClearFrameBuffer(TargetBuffer);
		mainshader->UpdateMV(ShadowingPointLights[SNum]->DirView, ShadowingPointLights[SNum]->Projection);
		UpdateGeometryShaderParams(ShadowingPointLights[SNum]->GetPosition(), ShadowingPointLights[SNum]->Projection);
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
		list->SetRenderTarget(nullptr);
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
		list->SetRenderTarget(nullptr);
	}
}

void ShadowRenderer::BindShadowMapsToTextures(RHICommandList * list)
{
	if (ShadowingDirectionalLights.size() > 0)
	{
		list->SetFrameBufferTexture(DirectionalLightBuffers[0], 4);
	}
	if (ShadowingDirectionalLights.size() > 1)
	{
		list->SetFrameBufferTexture(DirectionalLightBuffers[1], 5);
	}
}

void ShadowRenderer::ClearShadowLights()
{
	ShadowingDirectionalLights.clear();
}

void ShadowRenderer::InitShadows(std::vector<Light*> lights, RHICommandList* list)
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
				ShadowingDirectionalLights.push_back(lights[i]);
			}
			else
			{
				std::cout << "Max Directional Shadows Hit" << std::endl;
				lights[i]->SetShadow(false);
			}
		}
	}
	list->CreatePipelineState(DirectionalLightShader);
}