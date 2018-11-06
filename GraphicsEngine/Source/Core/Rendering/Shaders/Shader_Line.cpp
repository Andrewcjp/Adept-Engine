
#include "Shader_Line.h"
#include "Core/Assets/ShaderComplier.h"

DECLARE_GLOBAL_SHADER_ARGS(Shader_Line, bool);
DECLARE_GLOBAL_SHADER_PERMIUTATION(Shader_Line_2D_ON, Shader_Line, bool, true);
DECLARE_GLOBAL_SHADER_PERMIUTATION(Shader_Line_2D_OFF, Shader_Line, bool, false);

Shader_Line::Shader_Line(DeviceContext* device, bool is2D) : Shader(Device)
{
	m_Shader = RHI::CreateShaderProgam();
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("TWOD_ONLY", is2D ? "1" : "0"));

	m_Shader->AttachAndCompileShaderFromFile("debugline_vs", EShaderType::SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("debugline_fs", EShaderType::SHADER_FRAGMENT);
}

Shader_Line::~Shader_Line()
{}

std::vector<Shader::VertexElementDESC> Shader_Line::GetVertexFormat()
{
	std::vector<Shader::VertexElementDESC> out;
	out.push_back(VertexElementDESC{ "POSITION", 0, FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	out.push_back(VertexElementDESC{ "NORMAL", 0, FORMAT_R32G32B32_FLOAT, 0, 12, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	return out;
}

std::vector<Shader::ShaderParameter> Shader_Line::GetShaderParameters()
{
	std::vector<Shader::ShaderParameter> Output;
	Output.push_back(ShaderParameter(ShaderParamType::CBV, 0, 0));
	return Output;
}