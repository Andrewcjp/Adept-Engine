#include "Shader_RTBase.h"
IMPLEMENT_GLOBAL_SHADER(Shader_RTBase);

Shader_RTBase::Shader_RTBase(DeviceContext* C) :Shader(C)
{
	m_Shader->AttachAndCompileShaderFromFile("Raytracing\\DefaultShaders", EShaderType::SHADER_RT_LIB);
}


Shader_RTBase::~Shader_RTBase()
{

}
