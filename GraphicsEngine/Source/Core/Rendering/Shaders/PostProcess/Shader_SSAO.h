#pragma once
#include "RHI/Shader.h"
struct ShaderData
{
	float radius = 0.5f;
	float bias = 0.025f;
	int kernelSize = 64;
	glm::mat4 projection;
	glm::mat4 view;
	glm::vec3 samples[64];
};
class Shader_SSAO :
	public Shader
{
public:
	DECLARE_GLOBAL_SHADER(Shader_SSAO);
	Shader_SSAO(DeviceContext* d);
	void Bind(RHICommandList * list);
	~Shader_SSAO();
	void RenderPlane();
	void UpdateOGLUniforms(Transform * t, Camera * c, std::vector<Light*> lights);
	void Resize(int width, int height);
	RHIBuffer* DataBuffer;
	ShaderData CurrentData;
private:
	
	int quad_vertexbuffer;
	BaseTexture* noisetex;
	const int NoiseTextureUnit = 10;
	std::vector<glm::vec3> ssaoKernel;
	int mwidth = 0;
	int mheight = 0;
	// Inherited via Shader


	// Inherited via Shader
	void UpdateD3D11Uniforms(Transform * t, Camera * c, std::vector<Light*> lights);

};
class Shader_SSAO_Merge :
	public Shader
{
public:
	DECLARE_GLOBAL_SHADER(Shader_SSAO_Merge);
	Shader_SSAO_Merge(DeviceContext* d);
};
