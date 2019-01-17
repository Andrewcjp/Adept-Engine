#pragma once
#include "RHI/Shader.h"
class Shader_ShadowSample : public Shader
{
public:
	DECLARE_GLOBAL_SHADER(Shader_ShadowSample);
	Shader_ShadowSample(DeviceContext * Context);
	~Shader_ShadowSample();
	std::vector<Shader::ShaderParameter> GetShaderParameters();
	std::vector<Shader::VertexElementDESC> GetVertexFormat();
	static const int ShadowSRV = 3;
	static const int PreSampleCBV = 4;
};

