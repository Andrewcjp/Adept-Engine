#pragma once
#include "../RHI/Shader.h"
class Shader_Deferred :
	public Shader
{
public:
	Shader_Deferred();
	~Shader_Deferred();

	std::vector<Shader::ShaderParameter> GetShaderParameters();
	std::vector<Shader::VertexElementDESC> GetVertexFormat();
	void RenderScreenQuad(RHICommandList * list);

private:
	RHIBuffer * VertexBuffer = nullptr;
};

