#include "ShaderProgramBase.h"

ShaderProgramBase::~ShaderProgramBase()
{

}

void ShaderProgramBase::ModifyCompileEnviroment(Shader_Define Define)
{
	Defines.push_back(Define);
}
