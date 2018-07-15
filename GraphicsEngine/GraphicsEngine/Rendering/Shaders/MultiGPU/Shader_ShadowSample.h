#pragma once
#include "RHI/Shader.h"
class Shader_ShadowSample : public Shader
{
public:
	Shader_ShadowSample(DeviceContext * Context);
	~Shader_ShadowSample();
	std::vector<Shader::ShaderParameter> GetShaderParameters();
	std::vector<Shader::VertexElementDESC> GetVertexFormat();
};

