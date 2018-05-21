#pragma once
#include "../RHI/Shader.h"
#define DEBUG_CUBEMAPS 0
class Shader_Skybox : public Shader
{
public:
	Shader_Skybox();
	void Init(FrameBuffer * Buffer, FrameBuffer * DepthSourceBuffer);
	virtual ~Shader_Skybox();
	void Render(class Shader_Main * mainshader, FrameBuffer * Buffer, FrameBuffer * DepthSourceBuffer);
	std::vector<Shader::ShaderParameter> GetShaderParameters();
	std::vector<Shader::VertexElementDESC> GetVertexFormat();
	BaseTexture* SkyBoxTexture;
#if DEBUG_CUBEMAPS
	FrameBuffer* test = nullptr;
#endif
private:
	int SkyboxTexture;
	class Renderable* CubeModel;
	class RHICommandList* List;

};

