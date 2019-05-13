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
	void SetSkyBox(BaseTextureRef tex);
	void Render(class SceneRenderer * SceneRender, RHICommandList* list, FrameBuffer * Buffer, FrameBuffer * DepthSourceBuffer);
	std::vector<ShaderParameter> GetShaderParameters();
	std::vector<Shader::VertexElementDESC> GetVertexFormat();
	BaseTextureRef SkyBoxTexture = nullptr;
#if DEBUG_CUBEMAPS
	FrameBuffer* test = nullptr;
#endif
private:
	class Mesh* CubeModel = nullptr;
	//class RHICommandList* List = nullptr;
};

