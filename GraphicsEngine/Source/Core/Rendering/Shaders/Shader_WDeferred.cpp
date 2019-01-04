#include "Shader_WDeferred.h"
#include "RHI/RHI.h"
#include "RHI/ShaderProgramBase.h"
DECLARE_GLOBAL_SHADER(Shader_WDeferred);
Shader_WDeferred::Shader_WDeferred(class DeviceContext* dev) :Shader(dev)
{
	m_Shader = RHI::CreateShaderProgam();
	m_Shader->AttachAndCompileShaderFromFile("Main_vs", EShaderType::SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("DeferredWrite_fs", EShaderType::SHADER_FRAGMENT);

}

Shader_WDeferred::~Shader_WDeferred()
{}
