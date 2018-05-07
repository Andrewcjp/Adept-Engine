#include "Stdafx.h"
#include "Shader_Compost.h"
#include "../RHI/DeviceContext.h"

Shader_Compost::Shader_Compost(DeviceContext * context)
{
	m_Shader = RHI::CreateShaderProgam();

	m_Shader->CreateShaderProgram();

	m_Shader->AttachAndCompileShaderFromFile("PostProcessBase_VS", SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("Compost_fs", SHADER_FRAGMENT);

	m_Shader->BuildShaderProgram();
	m_Shader->ActivateShaderProgram();
}

Shader_Compost::~Shader_Compost()
{}
std::vector<Shader::ShaderParameter> Shader_Compost::GetShaderParameters()
{
	std::vector<Shader::ShaderParameter> out;
	out.push_back(ShaderParameter(ShaderParamType::SRV, 0, 0));
	return out;
}
std::vector<Shader::VertexElementDESC> Shader_Compost::GetVertexFormat()
{
	std::vector<VertexElementDESC> out;
	out.push_back(VertexElementDESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	return out;
}