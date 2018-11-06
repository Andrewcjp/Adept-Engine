
#include "Shader_UpSample.h"


Shader_UpSample::Shader_UpSample()
{
	m_Shader = RHI::CreateShaderProgam(RHI::GetDeviceContext(0));
	
	m_Shader->AttachAndCompileShaderFromFile("UpSampleCS", EShaderType::SHADER_COMPUTE);
}


Shader_UpSample::~Shader_UpSample()
{}
bool Shader_UpSample::IsComputeShader()
{
	return true;
}

std::vector<Shader::ShaderParameter> Shader_UpSample::GetShaderParameters()
{
	std::vector<Shader::ShaderParameter> Output;
	Output.push_back(ShaderParameter(ShaderParamType::SRV, 0, 0));
	Output.push_back(ShaderParameter(ShaderParamType::UAV, 1, 0));
	return Output;
}
