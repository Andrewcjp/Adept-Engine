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
IMPLEMENT_GLOBAL_SHADER(Shader_Skybox);
Shader_Skybox::Shader_Skybox(class DeviceContext* dev) :Shader(dev)
{
	MeshLoader::FMeshLoadingSettings settings = {};
	settings.Scale = glm::vec3(100.0f);
	CubeModel = RHI::CreateMesh("models\\SkyBoxCube.obj", settings);

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
	desc.DepthCompareFunction = COMPARISON_FUNC::COMPARISON_FUNC_LESS_EQUAL;
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
	list->GetDevice()->GetTimeManager()->StartTimer(list, EGPUTIMERS::Skybox);

	if (DepthSourceBuffer != nullptr)
	{
		RHIRenderPassDesc D = RHIRenderPassDesc(Buffer);
		D.FinalState = GPU_RESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		D.DepthSourceBuffer = DepthSourceBuffer;
		list->BeginRenderPass(D);
	}
	else
	{
		if (!Cubemap)
		{
			RHIRenderPassDesc D = RHIRenderPassDesc(Buffer);
			D.FinalState = GPU_RESOURCE_STATES::RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
			list->BeginRenderPass(D);
		}
	}
#if DEBUG_CUBEMAPS
	list->SetFrameBufferTexture(test, 0);
#else
	list->SetTexture(SkyBoxTexture, 0);
#endif
	if (!Cubemap)
	{
		SceneRender->BindMvBuffer(list, 1);
	}
	else
	{
		Cubemap->BindViews(list, index, 1);
	}
	CubeModel->Render(list);
	list->GetDevice()->GetTimeManager()->EndTimer(list, EGPUTIMERS::Skybox);
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
		if (list->GetDeviceIndex() == 1)
		{
			list->GetDevice()->GetTimeManager()->EndTotalGPUTimer(list);
		}
		//if (RHI::GetMGPUSettings()->MainPassSFR && list->GetDeviceIndex() == 0)
		//{
		//	list->InsertGPUStallTimer();
		//}
		//Buffer->MakeReadyForComputeUse(list);
		//List->Execute();
	}
	}

std::vector<ShaderParameter> Shader_Skybox::GetShaderParameters()
{
	std::vector<ShaderParameter> out;
	out.push_back(ShaderParameter(ShaderParamType::SRV, 0, 0));
	out.push_back(ShaderParameter(ShaderParamType::CBV, 1, 0));
	return out;
}

std::vector<Shader::VertexElementDESC> Shader_Skybox::GetVertexFormat()
{
	std::vector<VertexElementDESC> out;
	out.push_back(VertexElementDESC{ "POSITION", 0, FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	out[0].Stride = sizeof(OGLVertex);
	return out;
}
