#pragma once
#include "../RHI/Shader.h"
class Shader_Bloom : public Shader
{
public:
	Shader_Bloom();
	~Shader_Bloom();
	bool IsComputeShader() override;
	virtual std::vector<ShaderParameter> GetShaderParameters() override;
};

