#include "ShadowRenderer.h"
#include "RHI/RHI.h"
#include "../Rendering/Shaders/Shader_Main.h"
#include "../D3D12/D3D12RHI.h"
ShadowRenderer::ShadowRenderer()
{

}


ShadowRenderer::~ShadowRenderer()
{
	for (size_t SNum = 0; SNum < ShadowShaders.size(); SNum++)
	{
		delete ShadowShaders[SNum];
	}
}

void ShadowRenderer::RenderShadowMaps(Camera * c, std::vector<Light*> lights, const std::vector<GameObject*>& ShadowObjects, CommandListDef* list, Shader_Main* mainshader)
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

	for (size_t SNum = 0; SNum < ShadowShaders.size(); SNum++)
	{
		ShadowShaders[SNum]->SetShaderActive(list);
		if (RHI::GetType() != RenderSystemD3D12)
		{
			ShadowShaders[SNum]->shadowbuffer->ClearBuffer();
		}

		if (RHI::GetType() == RenderSystemD3D12)
		{
			mainshader->UpdateMV(ShadowShaders[0]->targetlight->DirView, ShadowShaders[0]->targetlight->Projection);
		}
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
			if (RHI::GetType() != RenderSystemD3D12)
			{
				ShadowShaders[SNum]->UpdateUniforms(ShadowObjects[i]->GetTransform(), c, lights);
				ShadowObjects[i]->Render(true, list);
			}
			else
			{
				mainshader->SetActiveIndex(list, (int)i);				
				ShadowObjects[i]->Render(false, list);
			}

		}
		ShadowShaders[SNum]->shadowbuffer->UnBind();
	}

}

void ShadowRenderer::BindShadowMaps(CommandListDef* list)
{
	for (size_t SNum = 0; SNum < ShadowShaders.size(); SNum++)
	{
		ShadowShaders[SNum]->BindShadowmmap(list);
	}
}
void ShadowRenderer::ClearShadowLights()
{
	for (size_t SNum = 0; SNum < ShadowShaders.size(); SNum++)
	{
		delete ShadowShaders[SNum];
	}
	ShadowShaders.clear();
}


void ShadowRenderer::InitShadows(std::vector<Light*> lights)
{
	int lastpointshadow = 0;
	int lastdirshadow = 0;
	for (size_t i = 0; i < lights.size(); i++)
	{
		if (lights[i]->GetDoesShadow() == false)
		{
			continue;
		}
		if (lights[i]->GetType() == Light::Directional)
		{
			if (lastdirshadow + 1 > MAX_DIRECTIONAL_SHADOWS)
			{
				lights[i]->SetShadow(false);
				std::cout << "Max Directional Shadows Hit" << std::endl;
				continue;
			}
			lights[i]->DirectionalShadowid = lastdirshadow;
			lastdirshadow++;
		}
		else
		{
			if (lastpointshadow + 1 > MAX_POINT_SHADOWS)
			{
				std::cout << "Max Point Shadows Hit" << std::endl;
				lights[i]->SetShadow(false);
				continue;
			}
			lights[i]->SetShadowId(lastpointshadow);
			lastpointshadow++;
		}
		if (lights[i]->GetType() == Light::Point)
		{
			ShadowShaders.push_back(new Shader_Depth(lights[i], true));
		}
		else
		{
			ShadowShaders.push_back(new Shader_Depth(lights[i], false));
		}
	}
}

CommandListDef * ShadowRenderer::CreateShaderCommandList()
{
	if (ShadowShaders.size() > 0)
	{
		return ((D3D12Shader*)ShadowShaders[0]->GetShaderProgram())->CreateShaderCommandList();
	}
	return nullptr;
}

void ShadowRenderer::ResetCommandList(CommandListDef * list)
{
	D3D12RHI::PreFrameSetUp(list, (D3D12Shader*)ShadowShaders[0]->GetShaderProgram());
}
