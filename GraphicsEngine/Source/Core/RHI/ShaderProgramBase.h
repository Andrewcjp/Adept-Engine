#pragma once
#include "RHI/ShaderBase.h"
#include "ShaderBase.h"
class ShaderProgramBase
{
protected:
	bool						IsCompute = false;
public:
	RHI_API ShaderProgramBase();
	RHI_API virtual								~ShaderProgramBase();
	RHI_API virtual EShaderError::Type				AttachAndCompileShaderFromFile(const char * filename, EShaderType::Type type);
	RHI_API virtual EShaderError::Type				AttachAndCompileShaderFromFile(const char * filename, EShaderType::Type type, const char * Entrypoint) = 0;

	struct Shader_Define
	{
		Shader_Define(std::string name, std::string value)
		{
			Name = name;
			Value = value;
		}
		std::string Name;
		std::string Value;
	};
	void										ModifyCompileEnviroment(Shader_Define Define);
	std::vector<ShaderParameter> GeneratedParams;
	
	bool IsComputeShader();
	bool IsRSResolved()const;
	void ResolveRS();
protected:
	bool HasResolved = false;
	void NumberRS();
	std::vector<Shader_Define> Defines;
};

