#pragma once
#include "RHI/ShaderProgramBase.h"
class NullRHIShaderProgram : public ShaderProgramBase
{
public:
	NullRHIShaderProgram(DeviceContext* dev);
	EShaderError::Type AttachAndCompileShaderFromFile(const char * filename, EShaderType::Type type, const char * Entrypoint) override;
	glm::ivec3 GetComputeThreadSize() const override;
};

