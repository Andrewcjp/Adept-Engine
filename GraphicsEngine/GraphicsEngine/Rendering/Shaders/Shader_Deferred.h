#pragma once
#include "../RHI/Shader.h"
class Shader_Deferred :
	public Shader
{
public:
	Shader_Deferred();
	~Shader_Deferred();



	// Inherited via Shader
	virtual void UpdateOGLUniforms(Transform * t, Camera * c, std::vector<Light*> lights) override;
	void RenderPlane();
private:
	GLuint quad_vertexbuffer;

	// Inherited via Shader
	virtual void UpdateD3D11Uniforms(Transform * t, Camera * c, std::vector<Light*> lights) override;
};

