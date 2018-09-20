#pragma once
#include "RHI/Shader.h"
class Shader_ParticleDraw :public Shader
{
public:
	Shader_ParticleDraw(class DeviceContext* device);
	~Shader_ParticleDraw();
private:
	std::vector<Shader::VertexElementDESC> GetVertexFormat()  override;
	std::vector<Shader::ShaderParameter> GetShaderParameters() override;

};

