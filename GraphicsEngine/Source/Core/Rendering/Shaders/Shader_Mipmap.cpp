#include "Shader_Mipmap.h"
#include "RHI\ShaderProgramBase.h"
#include "RHI\ShaderBase.h"
#include "RHI\Shader.h"

IMPLEMENT_GLOBAL_SHADER(Shader_Mipmap);
Shader_Mipmap::Shader_Mipmap(DeviceContext* device) :Shader(device)
{
	m_Shader->AttachAndCompileShaderFromFile("MipmapCS", EShaderType::SHADER_COMPUTE);
}


Shader_Mipmap::~Shader_Mipmap()
{}

std::vector<ShaderParameter> Shader_Mipmap::GetShaderParameters()
{
	m_Shader->GeneratedParams[2].Type = ShaderParamType::RootConstant;
	m_Shader->GeneratedParams[2].NumDescriptors = 2;
	return Shader::GetShaderParameters();
}

