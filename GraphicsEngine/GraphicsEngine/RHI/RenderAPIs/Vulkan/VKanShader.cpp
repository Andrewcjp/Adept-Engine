#include "stdafx.h"
#include "VKanShader.h"
#if BUILD_VULKAN

VKanShader::VKanShader()
{}


VKanShader::~VKanShader()
{}

void VKanShader::CreateShaderProgram()
{}

EShaderError VKanShader::AttachAndCompileShaderFromFile(const char * filename, EShaderType type)
{
	return EShaderError();
}

void VKanShader::BuildShaderProgram()
{}

void VKanShader::DeleteShaderProgram()
{}

void VKanShader::ActivateShaderProgram()
{}

void VKanShader::DeactivateShaderProgram()
{}
#endif