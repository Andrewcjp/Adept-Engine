#pragma once
#include "RHI\Shader.h"
class Shader_Compost :
	public Shader
{
public:
	DECLARE_GLOBAL_SHADER(Shader_Compost);
	Shader_Compost(class DeviceContext* context);
	virtual ~Shader_Compost();
	std::vector<Shader::ShaderParameter> GetShaderParameters() override;
	std::vector<Shader::VertexElementDESC> GetVertexFormat() override;
};

