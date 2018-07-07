#pragma once
#include "../RHI/Shader.h"
class Shader_EnvMap : public Shader
{
public:
	Shader_EnvMap();
	~Shader_EnvMap();
	std::vector<ShaderParameter> GetShaderParameters() override;
	std::vector<VertexElementDESC> GetVertexFormat() override;
	BaseTexture* TargetCubemap = nullptr;
	FrameBuffer * CubeBuffer = nullptr;
	void Init();
	void ProcessTexture(BaseTexture* target);
private:
	RHICommandList * CmdList = nullptr;
	RHIBuffer* ShaderData = nullptr;

	struct SData
	{
		glm::mat4x4 VP;
		int faceID = 0;
	};
	SData Data[6];
	Renderable* Cube = nullptr;
};

