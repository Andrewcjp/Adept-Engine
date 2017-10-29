#pragma once
#include "../RHI/Shader.h"
class Shader_Querry :
	public Shader
{
public:
	Shader_Querry();
	~Shader_Querry();

	// Inherited via Shader
	virtual void UpdateOGLUniforms(Transform * t, Camera * c, std::vector<Light*> lights) override;
private:
	int						m_uniform_model;
	int						m_UniformMVP;

	// Inherited via Shader
	virtual void UpdateD3D11Uniforms(Transform * t, Camera * c, std::vector<Light*> lights) override;
};

