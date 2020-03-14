#pragma once
#include "RHI/ShaderBase.h"
#include "ShaderBase.h"
class ShaderProgramBase
{
protected:
	bool						IsCompute = false;
public:
	RHI_API ShaderProgramBase(DeviceContext* context);
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
	static void AddDefaultDefines(std::vector<ShaderProgramBase::Shader_Define>& Defines, DeviceContext * context);
	void										ModifyCompileEnviroment(Shader_Define Define);
	std::vector<ShaderParameter> GeneratedParams;
	
	bool IsComputeShader();
	bool IsRSResolved()const;
	void ResolveRS();
	virtual glm::ivec3 GetComputeThreadSize()const;
	uint32 GetInstructionCount() const { return InstructionCount; }
	std::vector<Shader_Define> Defines;
protected:
	bool HasResolved = false;
	void NumberRS();
	uint32 InstructionCount = 0;
	friend class ShaderReflection;
	DeviceContext* Context;
};

