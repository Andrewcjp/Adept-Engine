#pragma once
#include "RHI\Shader.h"

class Shader_DebugOutput : public Shader
{
public:
	DECLARE_GLOBAL_SHADER(Shader_DebugOutput);
	Shader_DebugOutput(DeviceContext* device);
	~Shader_DebugOutput();
	virtual std::vector<ShaderParameter> GetShaderParameters() override;
};

