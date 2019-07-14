#pragma once
#include "RHI/Shader.h"

class ReflectionProbe;
class SceneRenderer;
#define DEBUG_CUBEMAPS 0
class Shader_Skybox : public Shader
{
public:
	DECLARE_GLOBAL_SHADER(Shader_Skybox);
	Shader_Skybox(class DeviceContext* dev);
	void Init(FrameBuffer * Buffer, FrameBuffer * DepthSourceBuffer);
	virtual ~Shader_Skybox();
	void SetSkyBox(BaseTextureRef tex);
	void Render(SceneRenderer * SceneRender, RHICommandList* list, FrameBuffer * Buffer, FrameBuffer * DepthSourceBuffer, ReflectionProbe* Cubemap = nullptr, int index = 0);
	std::vector<ShaderParameter> GetShaderParameters();
	std::vector<Shader::VertexElementDESC> GetVertexFormat();
	BaseTextureRef SkyBoxTexture;
#if DEBUG_CUBEMAPS
	FrameBuffer* test = nullptr;
#endif
private:
	class Mesh* CubeModel = nullptr;
	//class RHICommandList* List = nullptr;
};

