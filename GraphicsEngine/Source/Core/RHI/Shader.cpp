#include "Shader.h"
#include "RHI.h"

Shader::Shader()
{}

Shader::Shader(DeviceContext * context)
{
	Device = context;
	m_Shader = RHI::CreateShaderProgam(context);
}

Shader::~Shader()
{
	SafeDelete(m_Shader);
}

ShaderProgramBase * Shader::GetShaderProgram()
{
	return m_Shader;
}

std::vector<Shader::ShaderParameter> Shader::GetShaderParameters()
{
	return std::vector<Shader::ShaderParameter>();
}

std::vector<Shader::VertexElementDESC> Shader::GetVertexFormat()
{
	std::vector<Shader::VertexElementDESC> out;
	out.push_back(VertexElementDESC{ "POSITION", 0, FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	out.push_back(VertexElementDESC{ "NORMAL", 0, FORMAT_R32G32B32_FLOAT, 0, 12, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	out.push_back(VertexElementDESC{ "TEXCOORD", 0,FORMAT_R32G32_FLOAT, 0, 24,INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

	return out;
}

bool Shader::IsComputeShader()
{
	return false;
}

const std::string Shader::GetName()
{
	return "_";
}