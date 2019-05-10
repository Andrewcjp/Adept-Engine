#pragma once
#include "RHI\Shader.h"

class DeviceContext;

class Shader_Mipmap : public Shader
{
public:
	DECLARE_GLOBAL_SHADER(Shader_Mipmap);
	Shader_Mipmap(DeviceContext* device);
	~Shader_Mipmap();

	virtual std::vector<ShaderParameter> GetShaderParameters() override;

};

