#include "Shader_Skybox.h"
#include "Core/EngineInc.h"
#include "Core/Platform/PlatformCore.h"
#include "Rendering/Core/Mesh.h"
#include "Rendering/Core/ParticleSystemManager.h"
#include "Rendering/Core/SceneRenderer.h"
#include "RHI/DeviceContext.h"
#include "RHI/RHI.h"
#include "RHI/RHI_inc.h"
#include "RHI/ShaderProgramBase.h"
#include "Shader_Main.h"
#include "../Core/ReflectionProbe.h"
#include "RHI/SFRController.h"
#include "../Core/Screen.h"
IMPLEMENT_GLOBAL_SHADER(Shader_Skybox);
Shader_Skybox::Shader_Skybox(class DeviceContext* dev) :Shader(dev)
{
	m_Shader->AttachAndCompileShaderFromFile("Skybox_vs", EShaderType::SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("Skybox_fs", EShaderType::SHADER_FRAGMENT);
}

void Shader_Skybox::Init(FrameBuffer* Buffer, FrameBuffer* DepthSourceBuffer)
{
	//List = RHI::CreateCommandList(ECommandListType::Graphics, Device);

}

Shader_Skybox::~Shader_Skybox()
{
	//	SafeRHIRefRelease(SkyBoxTexture);
	EnqueueSafeRHIRelease(CubeModel);
}

void Shader_Skybox::SetSkyBox(BaseTextureRef tex)
{
	if (CubeModel == nullptr)
	{
		MeshLoader::FMeshLoadingSettings settings = {};
		settings.Scale = glm::vec3(100.0f);
		CubeModel = RHI::CreateMesh("models\\SkyBoxCube.obj", settings);
	}
	ensure(tex->GetType() == ETextureType::Type_CubeMap);
	if (SkyBoxTexture == tex)
	{
		return;
	}
	SkyBoxTexture = tex;
}

void Shader_Skybox::Render(class SceneRenderer * SceneRender, RHICommandList* list, FrameBuffer * Buffer, FrameBuffer * DepthSourceBuffer, ReflectionProbe* Cubemap /*= nullptr*/, int index /*= 0*/)
{
	if (RHI::IsVulkan())
	{
		return;
	}
	RHIPipeLineStateDesc desc;
	desc.DepthStencilState.DepthWrite = false;
	desc.Cull = false;
	desc.DepthStencilState.DepthCompareFunction = COMPARISON_FUNC::COMPARISON_FUNC_LESS_EQUAL;
	desc.ShaderInUse = this;
	desc.RenderTargetDesc = Buffer->GetPiplineRenderDesc();
	if (DepthSourceBuffer != nullptr)
	{
		desc.RenderTargetDesc = Buffer->GetPiplineRenderDesc();
		desc.RenderTargetDesc.DSVFormat = DepthSourceBuffer->GetPiplineRenderDesc().DSVFormat;
		list->SetPipelineStateDesc(desc);
	}
	else
	{
		list->SetPipelineStateDesc(desc);
	}
	
	{
		DECALRE_SCOPEDGPUCOUNTER(list, "SkyBox render");
		if (DepthSourceBuffer != nullptr)
		{
			RHIRenderPassDesc D = RHIRenderPassDesc(Buffer);
			D.FinalState = EResourceState::PixelShader;
			D.DepthSourceBuffer = DepthSourceBuffer;
			list->BeginRenderPass(D);
		}
		else
		{
			if (!Cubemap)
			{
				RHIRenderPassDesc D = RHIRenderPassDesc(Buffer);
				D.FinalState = EResourceState::PixelShader;
				list->BeginRenderPass(D);
			}
		}
		if (RHI::GetRenderSettings()->GetCurrnetSFRSettings().Enabled)
		{
			list->SetScissorRect(SFRController::GetScissor(list->GetDeviceIndex(), Screen::GetScaledRes()));
		}
#if DEBUG_CUBEMAPS
		list->SetFrameBufferTexture(test, "g_texture");
#else
		list->SetTexture(SkyBoxTexture, "g_texture");
#endif
		if (!Cubemap)
		{
			SceneRender->BindMvBuffer(list, "", 0);
		}
		else
		{
			Cubemap->BindViews(list, index, 1);
		}
		CubeModel->Render(list);
	}
	
	if (!Cubemap)
	{
		list->EndRenderPass();
	}
	//Buffer->MakeReadyForComputeUse(list);
	if (!Cubemap && false)
	{

		//Buffer->MakeReadyForComputeUse(List);
		if (list->GetDeviceIndex() == 0)
		{
			Buffer->MakeReadyForCopy(list);
		}
		//if (list->GetDeviceIndex() == 1)
		//{
		//	list->GetDevice()->GetTimeManager()->EndTotalGPUTimer(list);
		//}
		//if (RHI::GetMGPUSettings()->MainPassSFR && list->GetDeviceIndex() == 0)
		//{
		//	list->InsertGPUStallTimer();
		//}
		//Buffer->MakeReadyForComputeUse(list);
		//List->Execute();
	}
}

std::vector<VertexElementDESC> Shader_Skybox::GetVertexFormat()
{
	std::vector<VertexElementDESC> out;
	out.push_back(VertexElementDESC{ "POSITION", 0, R32G32B32_FLOAT, 0, 0, EInputClassification::PER_VERTEX, 0 });
	out[0].Stride = sizeof(OGLVertex);
	return out;
}
