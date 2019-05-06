#include "Shader_Skybox.h"
#include "RHI/RHI.h"
#include "RHI/RHI_inc.h"
#include "RHI/ShaderProgramBase.h"
#include "Rendering/Core/Mesh.h"
#include "Shader_Main.h"
#include "RHI/DeviceContext.h"
#include "Core/EngineInc.h"
#include "Rendering/Core/SceneRenderer.h"
#include "Core/Platform/PlatformCore.h"
#include "../Core/ParticleSystemManager.h"
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
	List = RHI::CreateCommandList(ECommandListType::Graphics,Device);
	RHIPipeLineStateDesc desc;
	desc.DepthStencilState.DepthWrite = false;
	desc.Cull = false;
	desc.DepthCompareFunction = COMPARISON_FUNC::COMPARISON_FUNC_LESS_EQUAL;
	desc.ShaderInUse = this;
	desc.FrameBufferTarget = Buffer;
	if (DepthSourceBuffer != nullptr)
	{		
		desc.RenderTargetDesc = Buffer->GetPiplineRenderDesc();
		desc.RenderTargetDesc.DSVFormat = DepthSourceBuffer->GetPiplineRenderDesc().DSVFormat;
		List->SetPipelineStateDesc(desc);
	}
	else
	{
		List->SetPipelineStateDesc(desc);
	}
}

Shader_Skybox::~Shader_Skybox()
{
	SafeRHIRefRelease(SkyBoxTexture);
	EnqueueSafeRHIRelease(CubeModel);
	EnqueueSafeRHIRelease(List);
}

void Shader_Skybox::SetSkyBox(BaseTexture * tex)
{
#if BASIC_RENDER_ONLY
	return;
#endif
	ensure(tex->GetType() == ETextureType::Type_CubeMap);
	if (SkyBoxTexture == tex)
	{
		return;
	}
	if (SkyBoxTexture != nullptr)
	{
		SafeRHIRefRelease(SkyBoxTexture);
	}
	SkyBoxTexture = tex;
	SkyBoxTexture->AddRef();
}

void Shader_Skybox::Render(SceneRenderer* SceneRender, FrameBuffer* Buffer, FrameBuffer* DepthSourceBuffer)
{
	List->ResetList();
	List->GetDevice()->GetTimeManager()->StartTimer(List, EGPUTIMERS::Skybox);
	if (DepthSourceBuffer != nullptr)
	{
		DepthSourceBuffer->BindDepthWithColourPassthrough(List, Buffer);
	}
	else
	{
		List->SetRenderTarget(Buffer);
	}
#if DEBUG_CUBEMAPS
	List->SetFrameBufferTexture(test, 0);
#else
	List->SetTexture(SkyBoxTexture, 0);
#endif
	SceneRender->BindMvBuffer(List, 1);
	CubeModel->Render(List);
	List->SetRenderTarget(nullptr);
	
	//Buffer->MakeReadyForComputeUse(List);
	if (List->GetDeviceIndex() == 0)
	{
		Buffer->MakeReadyForCopy(List);
	}
	List->GetDevice()->GetTimeManager()->EndTimer(List, EGPUTIMERS::Skybox);
	if (List->GetDeviceIndex() == 1)
	{
		List->GetDevice()->GetTimeManager()->EndTotalGPUTimer(List);
	}
	if (RHI::GetMGPUSettings()->MainPassSFR && List->GetDeviceIndex() == 0)
	{
		List->InsertGPUStallTimer();
	}
	List->Execute();
}

std::vector<Shader::ShaderParameter> Shader_Skybox::GetShaderParameters()
{
	std::vector<Shader::ShaderParameter> out;
	out.push_back(ShaderParameter(ShaderParamType::SRV, 0, 0));
	out.push_back(ShaderParameter(ShaderParamType::CBV, 1, 0));
	return out;
}

std::vector<Shader::VertexElementDESC> Shader_Skybox::GetVertexFormat()
{
	std::vector<VertexElementDESC> out;
	out.push_back(VertexElementDESC{ "POSITION", 0, FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	return out;
}
