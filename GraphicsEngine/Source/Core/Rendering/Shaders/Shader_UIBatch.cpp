
#include "Shader_UIBatch.h"
#include "Shader_Main.h"
IMPLEMENT_GLOBAL_SHADER(Shader_UIBatch);
Shader_UIBatch::Shader_UIBatch(class DeviceContext* dev) :Shader(dev)
{
	m_Shader = RHI::CreateShaderProgam();
	if (m_Shader == nullptr)
	{
		return;
	}


	m_Shader->AttachAndCompileShaderFromFile("UI_Batch_vs", EShaderType::SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("UI_Batch_fs", EShaderType::SHADER_FRAGMENT);

	UniformBuffer = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
	UniformBuffer->CreateConstantBuffer(sizeof(UnifromData), 1);
}

std::vector<Shader::VertexElementDESC> Shader_UIBatch::GetVertexFormat()
{
	std::vector<Shader::VertexElementDESC> out;
	///(float2 position : POSITION, int back : NORMAL0, float3 fc : TEXCOORD, float3 bc : TEXCOORD1)
	out.push_back(VertexElementDESC{ "POSITION", 0, FORMAT_R32G32_FLOAT, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	out.push_back(VertexElementDESC{ "TEXCOORD", 2, FORMAT_R16_UINT, 0, 8, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	out.push_back(VertexElementDESC{ "TEXCOORD", 0, FORMAT_R32G32B32_FLOAT, 0, 12,INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	out.push_back(VertexElementDESC{ "TEXCOORD", 1, FORMAT_R32G32B32_FLOAT, 0, 24, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	return out;
}

Shader_UIBatch::~Shader_UIBatch()
{
	EnqueueSafeRHIRelease(UniformBuffer);
}

void Shader_UIBatch::PushTOGPU(RHICommandList* list)
{
	list->SetConstantBufferView(UniformBuffer, 0, 0);
}

std::vector<Shader::ShaderParameter> Shader_UIBatch::GetShaderParameters()
{
	std::vector<Shader::ShaderParameter> Output;
	Shader_Main::GetMainShaderSig(Output);
	return Output;
}

void Shader_UIBatch::UpdateUniforms(glm::mat4x4 Proj)
{
	data.Proj = Proj;
	UniformBuffer->UpdateConstantBuffer(&data, 0);
}

