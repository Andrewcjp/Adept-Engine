#include "ShadowRenderer.h"
#include "RHI/RHI.h"
#include "../Rendering/Shaders/Shader_Main.h"
#include "../RHI/RenderAPIs/D3D12/D3D12RHI.h"
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
	for (int i = 0; i < MAX_POINT_SHADOWS; i++)
	{
		PointLightBuffers.push_back(RHI::CreateFrameBuffer(shadowwidth, shadowwidth, nullptr, 1, FrameBuffer::CubeDepth));
	}

	GeometryProjections = RHI::CreateRHIBuffer(RHIBuffer::Constant,nullptr);
	GeometryProjections->CreateConstantBuffer(sizeof(glm::mat4)*6, MAX_POINT_SHADOWS);
	PointShadowList = RHI::CreateCommandList();
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
		glm::lookAtRH(lightPos, lightPos + glm::vec3(0.0, 0.0,1.0), glm::vec3(0.0, 1.0, 0.0)));

	GeometryProjections->UpdateConstantBuffer(transforms, index);
}

void ShadowRenderer::RenderShadowMaps(Camera * c, std::vector<Light*>& lights, const std::vector<GameObject*>& ShadowObjects, RHICommandList* list, Shader_Main* mainshader)
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
		FrameBuffer* TargetBuffer = PointLightBuffers[SNum];

		list->SetRenderTarget(TargetBuffer);
		list->ClearFrameBuffer(TargetBuffer);		
		UpdateGeometryShaderParams(ShadowingPointLights[SNum]->GetPosition(), ShadowingPointLights[SNum]->Projection,SNum);
		list->SetConstantBufferView(GeometryProjections, SNum,0);
		Shader_Depth::LightData data = {};
		data.Proj = ShadowingPointLights[SNum]->Projection;
		data.Lightpos = ShadowingPointLights[SNum]->GetPosition();
		PointLightShader->UpdateBuffer(list, &data,SNum);
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
#include "../RHI/RenderAPIs/D3D12/D3D12Framebuffer.h"
#include "../RHI/RenderAPIs/D3D12/DescriptorHeap.h"
#include "../RHI/RenderAPIs/D3D12/D3D12CommandList.h"
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
	
	//list->SetFrameBufferTexture(PointLightBuffers[1], 5);
	//TODO: d3d12 Speccode!
	//hacky hack!
	//bind a heap with two srvs in
	//list->SetFrameBufferTexture(PointLightBuffers[1], 5);
	D3D12FrameBuffer* FB = (D3D12FrameBuffer*)PointLightBuffers[0];
	FB->CreateSRVHeap(ShadowingPointLights.size());
	DescriptorHeap* heap =  FB->GetHeap();
	for (int i = 0; i < ShadowingPointLights.size(); i++)
	{
		((D3D12FrameBuffer*)PointLightBuffers[i])->CreateSRVInHeap(i, heap);
		((D3D12FrameBuffer*)PointLightBuffers[i])->ReadyResourcesForRead(((D3D12CommandList*)list)->GetCommandList());
	}
	list->SetFrameBufferTexture(PointLightBuffers[0], 5);
#endif
}

void ShadowRenderer::ClearShadowLights()
{
	ShadowingDirectionalLights.clear();
	ShadowingPointLights.clear();
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
				lights[i]->SetShadowId(ShadowingPointLights.size());
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
				lights[i]->SetShadowId(ShadowingDirectionalLights.size());
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
	PointShadowList->SetPipelineState(PipeLineState{ true,false,false });
	PointShadowList->CreatePipelineState(PointLightShader, PointLightBuffers[0]);
}