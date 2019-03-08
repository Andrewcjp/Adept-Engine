#include "ShaderProgramBase.h"

ShaderProgramBase::~ShaderProgramBase()
{

}

EShaderError::Type ShaderProgramBase::AttachAndCompileShaderFromFile(const char * filename, EShaderType::Type type)
{
	return AttachAndCompileShaderFromFile(filename,type,"main");
}

void ShaderProgramBase::ModifyCompileEnviroment(Shader_Define Define)
{
	Defines.push_back(Define);
}
