
#include "Shader_TexturedUI.h"
#include "RHI/RHI.h"
#include "RHI/RHICommandList.h"

IMPLEMENT_GLOBAL_SHADER(Shader_TexturedUI);
Shader_TexturedUI::Shader_TexturedUI(DeviceContext* dev) :Shader(dev)
{
	m_Shader->AttachAndCompileShaderFromFile("PostProcess\\PostProcessBase_VS", EShaderType::SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("PostProcess\\Compost_fs", EShaderType::SHADER_FRAGMENT);
	if (!Engine::GetIsCooking())
	{
		Init();
		list = RHI::CreateCommandList(ECommandListType::Graphics, dev);
		RHIPipeLineStateDesc desc;
		desc.ShaderInUse = this;
		desc.InitOLD(false, false, true);
		desc.BlendState = RHIBlendState::CreateBlendDefault();
		desc.RenderTargetDesc = RHIPipeRenderTargetDesc::GetDefault();
		BlendPSO = RHI::CreatePipelineStateObject(desc);	
		NoBlendPSO = RHI::CreatePipelineStateObject(desc);
	}
}

void Shader_TexturedUI::Init()
{
	float g_quad_vertex_buffer_data[] = {
		-1.0f, -1.0f, 0.0f,0.0f,
		1.0f, -1.0f, 0.0f,0.0f,
		-1.0f,  1.0f, 0.0f,0.0f,
		-1.0f,  1.0f, 0.0f,0.0f,
		1.0f, -1.0f, 0.0f,0.0f,
		1.0f,  1.0f, 0.0f,0.0f,
	};
	VertexBuffer = RHI::CreateRHIBuffer(ERHIBufferType::Vertex);
	VertexBuffer->CreateVertexBuffer(sizeof(float) * 4, sizeof(float) * 6 * 4);
	VertexBuffer->UpdateVertexBuffer(&g_quad_vertex_buffer_data, sizeof(float) * 6 * 4);
}

Shader_TexturedUI::~Shader_TexturedUI()
{
	EnqueueSafeRHIRelease(list);
	EnqueueSafeRHIRelease(VertexBuffer);
	SafeRelease(NoBlendPSO);
	SafeRelease(BlendPSO);
}

void Shader_TexturedUI::Render()
{
	list->ResetList();
	if (blend)
	{
		list->SetPipelineStateObject(BlendPSO);
	}
	else
	{
		list->SetPipelineStateObject(NoBlendPSO);
	}
	list->BeginRenderPass(RHI::GetRenderPassDescForSwapChain());
	list->SetTexture(Texture, 0);
	list->SetVertexBuffer(VertexBuffer);
	list->DrawPrimitive(6, 1, 0, 0);
	list->EndRenderPass();
	list->Execute();
}

std::vector<ShaderParameter> Shader_TexturedUI::GetShaderParameters()
{
	std::vector<ShaderParameter> out;
	out.push_back(ShaderParameter(ShaderParamType::SRV, 0, 0));
	return out;
}
std::vector<Shader::VertexElementDESC> Shader_TexturedUI::GetVertexFormat()
{
	std::vector<VertexElementDESC> out;
	out.push_back(VertexElementDESC{ "POSITION", 0, FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	return out;
}
