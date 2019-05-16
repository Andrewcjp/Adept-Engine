
#include "Shader_DebugOutput.h"
IMPLEMENT_GLOBAL_SHADER(Shader_DebugOutput);

Shader_DebugOutput::Shader_DebugOutput(DeviceContext* device) :Shader(device)
{
	m_Shader->AttachAndCompileShaderFromFile("Deferred_LightingPass_vs", EShaderType::SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("Debug\\Debug_Render", EShaderType::SHADER_FRAGMENT);
}


Shader_DebugOutput::~Shader_DebugOutput()
{}

std::vector<ShaderParameter> Shader_DebugOutput::GetShaderParameters()
{
	std::vector<ShaderParameter> Output;
	Output.push_back(ShaderParameter(ShaderParamType::SRV, 0, 0));
	Output.push_back(ShaderParameter(ShaderParamType::RootConstant, 1, 0));
	return Output;
}
