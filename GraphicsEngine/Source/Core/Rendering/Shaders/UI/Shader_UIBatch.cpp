
#include "Shader_UIBatch.h"
#include "..\Shader_Main.h"

IMPLEMENT_GLOBAL_SHADER(Shader_UIBatch);
DECLARE_GLOBAL_SHADER_PERMIUTATION(Shader_UIBatch_Normal, Shader_UIBatch, int, 0, nullptr);
DECLARE_GLOBAL_SHADER_PERMIUTATION(Shader_UIBatch_Textured, Shader_UIBatch, int, 1, nullptr);

Shader_UIBatch::Shader_UIBatch(DeviceContext* dev, int Mode) :Shader(dev)
{
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("USE_TEXTURED", std::to_string(Mode)));
	m_Shader->AttachAndCompileShaderFromFile("UI_Batch_vs", EShaderType::SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("UI_Batch_fs", EShaderType::SHADER_FRAGMENT);
	this->Mode = Mode;
	UniformBuffer = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
	UniformBuffer->CreateConstantBuffer(sizeof(UnifromData), 1);
}

std::vector<VertexElementDESC> Shader_UIBatch::GetVertexFormat()
{
	std::vector<VertexElementDESC> out;
	///(float2 position : POSITION, int back : NORMAL0, float3 fc : TEXCOORD, float3 bc : TEXCOORD1)
	out.push_back(VertexElementDESC{ "POSITION", 0, R32G32_FLOAT, 0, 0, EInputClassification::PER_VERTEX, 0 });
	out.push_back(VertexElementDESC{ "TEXCOORD", 2, R16_UINT, 0, 8, EInputClassification::PER_VERTEX, 0 });
	out.push_back(VertexElementDESC{ "TEXCOORD", 0, R32G32B32_FLOAT, 0, 12,EInputClassification::PER_VERTEX, 0 });
	out.push_back(VertexElementDESC{ "TEXCOORD", 1, R32G32B32_FLOAT, 0, 24, EInputClassification::PER_VERTEX, 0 });
	return out;
}

Shader_UIBatch::~Shader_UIBatch()
{
	EnqueueSafeRHIRelease(UniformBuffer);
}

void Shader_UIBatch::PushTOGPU(RHICommandList* list)
{
	list->SetConstantBufferView(UniformBuffer, 0, "constantBuffer");
}

void Shader_UIBatch::UpdateUniforms(glm::mat4x4 Proj)
{
	data.Proj = Proj;
	UniformBuffer->UpdateConstantBuffer(&data, 0);
}

const std::string Shader_UIBatch::GetName()
{
	return "Shader_UIBatch" + std::to_string(Mode);
}

