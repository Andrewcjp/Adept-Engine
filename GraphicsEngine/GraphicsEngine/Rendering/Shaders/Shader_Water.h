#pragma once
#include "../RHI/Shader.h"
#include <string>
#define CUBE_SIDES 6
//clone of the main shader however with the geo shader and buffer.
class Shader_Water: public Shader
{
public:
	void UpdateOGLUniforms(Transform* t, Camera* c, std::vector<Light*> lights) override;
	Shader_Water();
	~Shader_Water();
	void SetShaderActive() override;
	void BindRefltBuffer();
	//int m_Model, m_farplane, m_lightpos = 0;
	int depthCubemap;
	int m_reflectBuffer;
	void BindShadowmmap();
private:
	int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
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

	// Inherited via Shader
	virtual void UpdateD3D11Uniforms(Transform * t, Camera * c, std::vector<Light*> lights) override;
};

