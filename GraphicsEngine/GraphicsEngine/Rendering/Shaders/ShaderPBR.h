#pragma once
#include "../RHI/Shader.h"

#include <string>
class ShaderPBR : public Shader
{
public:
	ShaderPBR();
	~ShaderPBR();
	void UpdateOGLUniforms(Transform* t, Camera* c, std::vector<Light*> lights) override;
private:
	int						m_uniform_model;
	int						m_uniform_View;
	int                     m_uniform_texture;
	int						m_UniformMVP;
	int						m_texDefaultSampler;
	int						m_uniform_LightNumber;
	int						m_DepthTexture;
	int						Uniform_Cam_Pos;
	int						m_IsMapUniform;
	int						m_FarPlane;
	int						m_MV33;
	int						IsReflect;

	// Inherited via Shader
	virtual void UpdateD3D11Uniforms(Transform * t, Camera * c, std::vector<Light*> lights) override;
};

