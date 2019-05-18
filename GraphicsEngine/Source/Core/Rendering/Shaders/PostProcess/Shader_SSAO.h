#pragma once
#include "RHI/Shader.h"
class Shader_SSAO :
	public Shader
{
public:
	DECLARE_GLOBAL_SHADER(Shader_SSAO);
	Shader_SSAO(DeviceContext* d);
	~Shader_SSAO();
	void RenderPlane();
	void UpdateOGLUniforms(Transform * t, Camera * c, std::vector<Light*> lights);
	void Resize(int width, int height);
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

