
#include "Shader_Compost.h"
#include "RHI/DeviceContext.h"
IMPLEMENT_GLOBAL_SHADER(Shader_Compost);
Shader_Compost::Shader_Compost(DeviceContext * context) :Shader(context)
{
	m_Shader->AttachAndCompileShaderFromFile("PostProcess\\PostProcessBase_VS", EShaderType::SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("PostProcess\\Compost_fs", EShaderType::SHADER_FRAGMENT);
}

Shader_Compost::~Shader_Compost()
{

}

std::vector<ShaderParameter> Shader_Compost::GetShaderParameters()
{
	std::vector<ShaderParameter> out;
	out.push_back(ShaderParameter(ShaderParamType::SRV, 0, 0));
	return out;
}
std::vector<VertexElementDESC> Shader_Compost::GetVertexFormat()
{
	std::vector<VertexElementDESC> out;
	out.push_back(VertexElementDESC{ "POSITION", 0, R32G32B32_FLOAT, 0, 0, EInputClassification::PER_VERTEX, 0 });
	out[0].Stride = sizeof(glm::vec4);
	return out;
}

IMPLEMENT_GLOBAL_SHADER(Shader_VROutput);
Shader_VROutput::Shader_VROutput(DeviceContext * context) :Shader(context)
{
	m_Shader->AttachAndCompileShaderFromFile("PostProcess\\PostProcessBase_VS", EShaderType::SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("PostProcess\\VrMirror_FS", EShaderType::SHADER_FRAGMENT);
}

Shader_VROutput::~Shader_VROutput()
{}

std::vector<ShaderParameter> Shader_VROutput::GetShaderParameters()
{
	std::vector<ShaderParameter> out;
	out.push_back(ShaderParameter(ShaderParamType::SRV, 0, 0));
	out.push_back(ShaderParameter(ShaderParamType::SRV, 1, 1));
	return out;
}

std::vector<VertexElementDESC> Shader_VROutput::GetVertexFormat()
{
	std::vector<VertexElementDESC> out;
	out.push_back(VertexElementDESC{ "POSITION", 0, R32G32B32_FLOAT, 0, 0, EInputClassification::PER_VERTEX, 0 });
	return out;
}
