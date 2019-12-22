#pragma once
#include "RHI\Shader.h"
//As this shader is a generation one it currently holds its own processing code. This might change later.
class Shader_Convolution :
	public Shader
{
public:
	DECLARE_GLOBAL_SHADER(Shader_Convolution);
	Shader_Convolution(class DeviceContext* dev);
	~Shader_Convolution();
	void init();
	void ComputeConvolution(BaseTextureRef Target, FrameBuffer* Buffer);
	void ComputeConvolutionProbe(RHICommandList* List, FrameBuffer* Target, FrameBuffer * Buffer);
	std::vector<ShaderParameter> GetShaderParameters() override;
	std::vector<VertexElementDESC> GetVertexFormat() override;
	BaseTextureRef TargetCubemap = nullptr;
	FrameBuffer * CubeBuffer = nullptr;
private:
	RHIPipeLineStateDesc ConvPSODesc;
	RHICommandList* CmdList = nullptr;
	RHIBuffer* ShaderData = nullptr;

	struct SData
	{
		glm::mat4x4 VP;
		int faceID = 0;
	};
	SData Data[6];
	Mesh* Cube = nullptr;
};

