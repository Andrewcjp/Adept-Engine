#pragma once
#include "RHI/Shader.h"
#include "Shader_Convolution.h"
class Shader_EnvMap : public Shader
{
public:
	DECLARE_GLOBAL_SHADER(Shader_EnvMap);
	Shader_EnvMap(class DeviceContext* dev);
	~Shader_EnvMap();
	std::vector<ShaderParameter> GetShaderParameters() override;
	std::vector<VertexElementDESC> GetVertexFormat() override;
	BaseTextureRef TargetCubemap = nullptr;
	FrameBuffer * CubeBuffer = nullptr;
	void Init();
	void ProcessTexture(BaseTextureRef target);
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
	Mesh* Cube = nullptr;
};

