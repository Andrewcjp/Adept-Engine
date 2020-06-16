
#include "Text_Shader.h"
#include "RHI/RHI.h"
#include "RHI/DeviceContext.h"
IMPLEMENT_GLOBAL_SHADER(Text_Shader);
Text_Shader::Text_Shader(DeviceContext* context):Shader(context)
{
	m_Shader->AttachAndCompileShaderFromFile("text_vs_Atlas", EShaderType::SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("text_fs_Atlas", EShaderType::SHADER_FRAGMENT);

	CBV = RHI::CreateRHIBuffer(ERHIBufferType::Constant, context);
	CBV->CreateConstantBuffer(sizeof(Data), 1);
}

std::vector<ShaderParameter> Text_Shader::GetShaderParameters()
{
	std::vector<ShaderParameter> out;
	out.resize(2);
	out[0] = ShaderParameter(ShaderParamType::SRV, 0, 0);
	out[1] = ShaderParameter(ShaderParamType::CBV, 1, 0);
	return out;
}
std::vector<VertexElementDESC> Text_Shader::GetVertexFormat()
{
	std::vector<VertexElementDESC> out;
	out.push_back(VertexElementDESC{ "POSITION", 0, R32G32B32A32_FLOAT, 0, 0, EInputClassification::PER_VERTEX, 0 });
	out.push_back(VertexElementDESC{ "NORMAL", 0, R32G32B32_FLOAT, 0, 16,EInputClassification::PER_VERTEX, 0 });
	return out;
}
void Text_Shader::Update(RHICommandList* lsit)
{
	Data.proj = glm::ortho(0.0f, (float)(Width), 0.0f, (float)(Height));
	CBV->UpdateConstantBuffer(&Data, 0);
	lsit->SetConstantBufferView(CBV, 0, 1);
}


Text_Shader::~Text_Shader()
{
	EnqueueSafeRHIRelease(CBV);
}

