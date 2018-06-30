#pragma once
#include "RHI\Shader.h"
class Shader_UpSample : public Shader
{
public:
	Shader_UpSample();
	~Shader_UpSample();
	bool IsComputeShader() override;
	virtual std::vector<ShaderParameter> GetShaderParameters() override;
};

