#pragma once
#include "RHI/Shader.h"
class Shader_Bloom : public Shader
{
public:
	DECLARE_GLOBAL_SHADER(Shader_Bloom);
	//Shader_Bloom() :Shader_Bloom(RHI::GetDefaultDevice()) {}
	Shader_Bloom(class DeviceContext* device);
	~Shader_Bloom();
	bool IsComputeShader() override;
	virtual std::vector<ShaderParameter> GetShaderParameters() override;
};

