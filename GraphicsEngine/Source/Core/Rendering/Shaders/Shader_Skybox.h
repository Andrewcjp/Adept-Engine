#pragma once
#include "RHI/Shader.h"
#define DEBUG_CUBEMAPS 0
class Shader_Skybox : public Shader
{
public:
	DECLARE_GLOBAL_SHADER(Shader_Skybox);
	Shader_Skybox(class DeviceContext* dev);
	void Init(FrameBuffer * Buffer, FrameBuffer * DepthSourceBuffer);
	virtual ~Shader_Skybox();
	void SetSkyBox(BaseTexture* tex);
	void Render(class SceneRenderer * SceneRender, FrameBuffer * Buffer, FrameBuffer * DepthSourceBuffer);
	std::vector<ShaderParameter> GetShaderParameters();
	std::vector<Shader::VertexElementDESC> GetVertexFormat();
	BaseTexture* SkyBoxTexture = nullptr;
#if DEBUG_CUBEMAPS
	FrameBuffer* test = nullptr;
#endif
private:
	class Mesh* CubeModel = nullptr;
	class RHICommandList* List = nullptr;
};

