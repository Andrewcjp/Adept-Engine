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
		DirectionalLightBuffers.push_back(RHI::CreateFrameBuffer(shadowwidth, shadowwidth,nullptr, 1, FrameBuffer::Depth));
	}
	PointLightBuffer = RHI::CreateFrameBuffer(shadowwidth, shadowwidth, nullptr, 1, FrameBuffer::CubeDepth);
	GeometryProjections = RHI::CreateRHIBuffer(RHIBuffer::Constant);
	GeometryProjections->CreateConstantBuffer(sizeof(glm::mat4)*6,1);
	PointShadowList = RHI::CreateCommandList();
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
	glm::mat4 transforms[6];
	transforms[0] = (shadowProj *
		glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0)));
	transforms[1] = (shadowProj *
		glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0)));
	transforms[2] = (shadowProj *
		glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
	transforms[3] = (shadowProj *
		glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
	transforms[4] = (shadowProj *
		glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, 1.0, 0.0)));
	transforms[5] = (shadowProj *
		glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, 1.0, 0.0)));

	GeometryProjections->UpdateConstantBuffer(transforms, 0);
}

void ShadowRenderer::RenderShadowMaps(Camera * c, std::vector<Light*> lights, const std::vector<GameObject*>& ShadowObjects, RHICommandList* list, Shader_Main* mainshader)
{
	//RenderDirectionalShadows(list, mainshader, ShadowObjects);

	PointShadowList->ResetList();
	RenderPointShadows(PointShadowList, mainshader, ShadowObjects);
	PointShadowList->Execute();
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
		list->SetConstantBufferView(GeometryProjections, 0, 2);
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
#if 0
	if (ShadowingDirectionalLights.size() > 0)
	{
		list->SetFrameBufferTexture(DirectionalLightBuffers[0], 4);
	}
	if (ShadowingDirectionalLights.size() > 1)
	{
		list->SetFrameBufferTexture(DirectionalLightBuffers[1], 5);
	}
#else 
	//3d texture unit!
	list->SetFrameBufferTexture(PointLightBuffer, 5);

#endif
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
	list->CreatePipelineState(DirectionalLightShader, DirectionalLightBuffer);
	PointShadowList->SetPipelineState(PipeLineState{ false,false,false });
	PointShadowList->CreatePipelineState(PointLightShader, PointLightBuffer);
}