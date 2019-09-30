#include "ShaderProgramBase.h"
#include <set>

ShaderProgramBase::ShaderProgramBase()
{
	if (!RHI::IsVulkan())
	{
		//the vulkan to HLSL complier defines this for us
		ModifyCompileEnviroment(Shader_Define("VULKAN", std::to_string(RHI::IsVulkan())));
	}
	ModifyCompileEnviroment(Shader_Define("DX12", std::to_string(RHI::IsD3D12())));
}

ShaderProgramBase::~ShaderProgramBase()
{

}

EShaderError::Type ShaderProgramBase::AttachAndCompileShaderFromFile(const char * filename, EShaderType::Type type)
{
	return AttachAndCompileShaderFromFile(filename, type, "main");
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

bool ShaderProgramBase::IsRSResolved() const
{
	return HasResolved;
}

void ShaderProgramBase::ResolveRS()
{
	//RemoveDupes();
	NumberRS();
	HasResolved = true;
}
