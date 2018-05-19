#pragma once
#include "../RHI/Shader.h"
class Shader_Line :
	public Shader
{
public:
	Shader_Line(bool is2D);
	virtual ~Shader_Line();
	std::vector<Shader::VertexElementDESC> GetVertexFormat() override;
	std::vector<Shader::ShaderParameter> GetShaderParameters() override;
};

