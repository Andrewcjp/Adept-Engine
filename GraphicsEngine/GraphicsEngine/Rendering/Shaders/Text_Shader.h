#pragma once
#include "../RHI/Shader.h"
class Text_Shader :
	public Shader
{
public:
	Text_Shader();
	std::vector<Shader::ShaderParameter> GetShaderParameters() override;
	std::vector<Shader::VertexElementDESC> GetVertexFormat() override;
	void Update(RHICommandList * lsit);
	virtual ~Text_Shader();

	// Inherited via Shader
	virtual void UpdateOGLUniforms(Transform * t, Camera * c, std::vector<Light*> lights) override;
	virtual void UpdateD3D11Uniforms(Transform * t, Camera * c, std::vector<Light*> lights) override;
	glm::vec3 Colour;
	int Width = 0;
	int Height = 0;
	struct ShaderData
	{
		glm::mat4 proj;
		glm::vec3 Colour;
	};
private:
	RHIBuffer * CBV = nullptr;
	ShaderData Data;
};

