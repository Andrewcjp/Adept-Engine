#include "Stdafx.h"
#include "Shader_FastBlur.h"

IMPLEMENT_GLOBAL_SHADER(Shader_FastBlur);

Shader_FastBlur::Shader_FastBlur(DeviceContext * con) :Shader(con)
{
	m_Shader->AttachAndCompileShaderFromFile("PostProcess\\FastBlurCS", EShaderType::SHADER_COMPUTE);
}

Shader_FastBlur::~Shader_FastBlur()
{}
