#include "Shader_WDeferred.h"
#include "RHI/RHI.h"
#include "RHI/ShaderProgramBase.h"

Shader_WDeferred::Shader_WDeferred()
{
	//Initialise OGL shader
	m_Shader = RHI::CreateShaderProgam();

	m_Shader->CreateShaderProgram();
	m_Shader->AttachAndCompileShaderFromFile("Main_vs_12", SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("DeferredWrite_fs", SHADER_FRAGMENT);

	m_Shader->BuildShaderProgram();
	m_Shader->ActivateShaderProgram();

}


Shader_WDeferred::~Shader_WDeferred()
{
}

void Shader_WDeferred::SetNormalState(bool hasnormalmap)
{


}
