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

void ShaderProgramBase::NumberRS()
{
	for (int i = 0; i < GeneratedParams.size(); i++)
	{
		GeneratedParams[i].SignitureSlot = i;
	}
}

bool ShaderProgramBase::IsComputeShader()
{
	return IsCompute;
}
