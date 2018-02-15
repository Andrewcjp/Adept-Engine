#include "stdafx.h"
#include "ShaderMipMap.h"


ShaderMipMap::ShaderMipMap()
{
	m_Shader = RHI::CreateShaderProgam();

	m_Shader->CreateShaderProgram();
	m_Shader->AttachAndCompileShaderFromFile("MipmapCS", SHADER_COMPUTE);

	m_Shader->BuildShaderProgram();
	m_Shader->ActivateShaderProgram();

}


ShaderMipMap::~ShaderMipMap()
{}
