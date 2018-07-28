#pragma once
#include "RHI/Shader.h"
#include "Shader_Convolution.h"
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
	void ComputeEnvBRDF();
	FrameBuffer * EnvBRDFBuffer = nullptr;
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
	Shader_Convolution::QuadDrawer* QuadDraw = nullptr;
};

