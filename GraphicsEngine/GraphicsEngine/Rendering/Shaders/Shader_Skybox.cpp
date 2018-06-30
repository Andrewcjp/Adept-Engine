#include "Shader_Skybox.h"
#include "RHI/RHI.h"
#include "RHI/ShaderProgramBase.h"
#include "Rendering/Core/Renderable.h"
#include "Shader_Main.h"
#include "RHI/RenderAPIs/D3D12/D3D12TimeManager.h"
#include "RHI/DeviceContext.h"
Shader_Skybox::Shader_Skybox()
{
	SkyBoxTexture = AssetManager::DirectLoadTextureAsset("\\asset\\texture\\cube_1024_preblurred_angle3_ArstaBridge.dds");
	MeshLoader::FMeshLoadingSettings settings = {};
	settings.Scale = glm::vec3(100.0f);
	CubeModel = RHI::CreateMesh("SkyBoxCube.obj", settings);
	m_Shader = RHI::CreateShaderProgam();
	m_Shader->CreateShaderProgram();
	m_Shader->AttachAndCompileShaderFromFile("Skybox_vs", SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("Skybox_fs", SHADER_FRAGMENT);
}
#include "RHI/RenderAPIs/D3D12/D3D12Framebuffer.h"
#include "RHI/RenderAPIs/D3D12/D3D12CommandList.h"
#include "../RHI/RenderAPIs/D3D12/GPUResource.h"
void Shader_Skybox::Init(FrameBuffer* Buffer, FrameBuffer* DepthSourceBuffer)
{
	List = RHI::CreateCommandList();
	PipeLineState state = {};
	state.DepthWrite = false;
	state.DepthCompareFunction = COMPARISON_FUNC::COMPARISON_FUNC_LESS_EQUAL;
	state.Cull = false;
	List->SetPipelineState(state);
	if (DepthSourceBuffer != nullptr)
	{
		D3D12Shader::PipeRenderTargetDesc Desc = ((D3D12FrameBuffer*)Buffer)->GetPiplineRenderDesc();
		Desc.DSVFormat = ((D3D12FrameBuffer*)DepthSourceBuffer)->GetPiplineRenderDesc().DSVFormat;
		((D3D12CommandList*)List)->CreatePipelineState(this, Desc);
	}
	else
	{
		List->CreatePipelineState(this, Buffer);
	}
}

Shader_Skybox::~Shader_Skybox()
{
	delete CubeModel;
	delete SkyBoxTexture;
}

void Shader_Skybox::Render(Shader_Main* mainshader, FrameBuffer* Buffer, FrameBuffer* DepthSourceBuffer)
{
	List->ResetList();
	List->GetDevice()->GetTimeManager()->StartTimer(List, D3D12TimeManager::eGPUTIMERS::Skybox);
	if (DepthSourceBuffer != nullptr)
	{
		if (RHI::IsD3D12())
		{
			D3D12FrameBuffer* fb = (D3D12FrameBuffer*)DepthSourceBuffer;
			D3D12CommandList* ll = (D3D12CommandList*)List;
			fb->BindDepthWithColourPassthrough(ll->GetCommandList(), (D3D12FrameBuffer*)Buffer);
		}
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
	mainshader->BindMvBuffer(List, 1);	
	CubeModel->Render(List);

	///todo:!
	D3D12FrameBuffer* dBuffer = (D3D12FrameBuffer*)Buffer;
	dBuffer->GetResource(0)->SetResourceState(((D3D12CommandList*)List)->GetCommandList(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	
	List->GetDevice()->GetTimeManager()->EndTimer(List, D3D12TimeManager::eGPUTIMERS::Skybox);
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
	out.push_back(VertexElementDESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	return out;
}
