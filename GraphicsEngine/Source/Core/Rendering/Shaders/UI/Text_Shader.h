#pragma once
#include "RHI/Shader.h"
class Text_Shader :
	public Shader
{
public:
	DECLARE_GLOBAL_SHADER(Text_Shader);
	Text_Shader(class DeviceContext* context);
	std::vector<ShaderParameter> GetShaderParameters() override;
	std::vector<VertexElementDESC> GetVertexFormat() override;
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

