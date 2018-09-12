#include "Shader_WDeferred.h"
#include "RHI/RHI.h"
#include "RHI/ShaderProgramBase.h"

Shader_WDeferred::Shader_WDeferred()
{
	m_Shader = RHI::CreateShaderProgam();	
	m_Shader->AttachAndCompileShaderFromFile("Main_vs", EShaderType::SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("DeferredWrite_fs", EShaderType::SHADER_FRAGMENT);

}

Shader_WDeferred::~Shader_WDeferred()
{
}
