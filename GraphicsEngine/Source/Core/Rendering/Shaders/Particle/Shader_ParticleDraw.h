#pragma once
#include "RHI/Shader.h"
class Shader_ParticleDraw :public Shader
{
public:
	DECLARE_GLOBAL_SHADER(Shader_ParticleDraw)
	Shader_ParticleDraw(class DeviceContext* device);
	~Shader_ParticleDraw();
	DEFINE_CACHESHADERPARAM(Texture);
	CACHED_SHADER_PARM(Texture,Tex);
	virtual void CacheParms() override;

private:
	std::vector<Shader::VertexElementDESC> GetVertexFormat()  override;
	std::vector<ShaderParameter> GetShaderParameters() override;

};

