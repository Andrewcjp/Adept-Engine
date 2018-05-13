#pragma once
#include "../RHI/Shader.h"

#include <vector>
#include <random>
#include "glm\glm.hpp"
class Shader_SSAO :
	public Shader
{
public:
	Shader_SSAO();
	~Shader_SSAO();
	void RenderPlane();
	virtual void UpdateOGLUniforms(Transform * t, Camera * c, std::vector<Light*> lights) override;
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
	virtual void UpdateD3D11Uniforms(Transform * t, Camera * c, std::vector<Light*> lights) override;

};

