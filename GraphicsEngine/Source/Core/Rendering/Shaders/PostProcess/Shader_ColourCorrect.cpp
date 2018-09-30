#include "Stdafx.h"
#include "Shader_ColourCorrect.h"
DECLARE_GLOBAL_SHADER(Shader_ColourCorrect);

Shader_ColourCorrect::Shader_ColourCorrect(DeviceContext* dev):Shader(dev)
{
	m_Shader = RHI::CreateShaderProgam();
	
	

	m_Shader->AttachAndCompileShaderFromFile("PostProcessBase_VS", EShaderType::SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("Pass_fs_12", EShaderType::SHADER_FRAGMENT);

	
	
}

Shader_ColourCorrect::~Shader_ColourCorrect()
{

}
std::vector<Shader::ShaderParameter> Shader_ColourCorrect::GetShaderParameters()
{
	std::vector<Shader::ShaderParameter> out;
	out.push_back(ShaderParameter(ShaderParamType::SRV, 0, 0));
	out.push_back(ShaderParameter(ShaderParamType::SRV, 1, 1));
	return out;
}
std::vector<Shader::VertexElementDESC> Shader_ColourCorrect::GetVertexFormat()
{
	std::vector<VertexElementDESC> out;
	out.push_back(VertexElementDESC{ "POSITION", 0, FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	return out;
}