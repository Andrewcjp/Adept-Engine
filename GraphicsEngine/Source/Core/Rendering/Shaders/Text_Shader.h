#pragma once
#include "RHI/Shader.h"
class Text_Shader :
	public Shader
{
public:
	Text_Shader(class DeviceContext* context);
	std::vector<Shader::ShaderParameter> GetShaderParameters() override;
	std::vector<Shader::VertexElementDESC> GetVertexFormat() override;
	void Update(RHICommandList * lsit);
	virtual ~Text_Shader();


	int Width = 0;
	int Height = 0;
	struct ShaderData
	{
		glm::mat4 proj;
	};
private:
	RHIBuffer * CBV = nullptr;
	ShaderData Data;
};

