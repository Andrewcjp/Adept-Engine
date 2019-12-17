#include "Shader_FXAA.h"

IMPLEMENT_GLOBAL_SHADER(Shader_FXAA);
Shader_FXAA::Shader_FXAA(DeviceContext* con) :Shader(con)
{
	m_Shader->AttachAndCompileShaderFromFile("PostProcess\\FXAA", EShaderType::SHADER_COMPUTE);
}

Shader_FXAA::~Shader_FXAA()
{}
