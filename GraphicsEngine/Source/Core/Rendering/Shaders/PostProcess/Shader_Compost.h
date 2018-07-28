#pragma once
#include "RHI\Shader.h"
class Shader_Compost :
	public Shader
{
public:
	Shader_Compost(class DeviceContext* context);
	virtual ~Shader_Compost();
	std::vector<Shader::ShaderParameter> GetShaderParameters() override;
	std::vector<Shader::VertexElementDESC> GetVertexFormat() override;
};

