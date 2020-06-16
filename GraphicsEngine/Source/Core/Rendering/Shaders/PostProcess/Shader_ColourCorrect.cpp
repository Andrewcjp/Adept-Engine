
#include "Shader_ColourCorrect.h"

IMPLEMENT_GLOBAL_SHADER(Shader_ColourCorrect);
Shader_ColourCorrect::Shader_ColourCorrect(DeviceContext* dev) :Shader(dev)
{
	m_Shader->AttachAndCompileShaderFromFile("PostProcess\\PostProcessBase_VS", EShaderType::SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("Pass_fs_12", EShaderType::SHADER_FRAGMENT);
}

Shader_ColourCorrect::~Shader_ColourCorrect()
{

}
std::vector<ShaderParameter> Shader_ColourCorrect::GetShaderParameters()
{
	std::vector<ShaderParameter> out;
	out.push_back(ShaderParameter(ShaderParamType::SRV, 0, 0));
	out.push_back(ShaderParameter(ShaderParamType::SRV, 1, 1));
	return out;
}
std::vector<VertexElementDESC> Shader_ColourCorrect::GetVertexFormat()
{
	std::vector<VertexElementDESC> out;
	out.push_back(VertexElementDESC{ "POSITION", 0, R32G32B32_FLOAT, 0, 0, EInputClassification::PER_VERTEX, 0 });
	return out;
}