#pragma once
#include "RHI/Shader.h"
class Shader_ShadowSample : public Shader
{
public:
	DECLARE_GLOBAL_SHADER_ARGS(Shader_ShadowSample,int);
	Shader_ShadowSample(DeviceContext * Context, int SampleCount);
	virtual ~Shader_ShadowSample();
	std::vector<Shader::VertexElementDESC> GetVertexFormat();
};

