#pragma once
#include "RHI/Shader.h"
#include "RHI/RHI.h"
class ShaderOutput :public Shader
{
public:

	ShaderOutput(int width = 1, int height = 1);
	void Resize(int width, int height);
	~ShaderOutput();

	
	void RenderPlane();
	void SetFXAA(bool state);
	void SetFullScreen(bool state);
	std::vector<ShaderParameter> GetShaderParameters() override;
	std::vector<VertexElementDESC> GetVertexFormat() override;
private:
	struct ConstantBuffer
	{
		glm::mat4		m_worldMat;
		glm::mat4		m_viewMat;
		glm::mat4		m_projection;
	};
	void UpdateD3D11Uniforms(Transform * t, Camera * c, std::vector<Light*> lights) override;
	void UpdateOGLUniforms(Transform* t, Camera* c, std::vector<Light*> lights) override;

	
	
	int						m_uniform_model;
	int						m_uniform_View;
	int						m_UniformMVP;
	int mwidth = 0;
	int mheight = 0;
	bool fxxastate = false;
	ConstantBuffer m_cbuffer;
};

