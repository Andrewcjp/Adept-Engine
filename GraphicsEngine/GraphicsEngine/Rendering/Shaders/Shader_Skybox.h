#pragma once
#include "../RHI/Shader.h"
class Shader_Skybox : public Shader
{
public:
	Shader_Skybox();
	virtual ~Shader_Skybox();

	// Inherited via Shader
	virtual void UpdateOGLUniforms(Transform * t, Camera * c, std::vector<Light*> lights = std::vector<Light*>()) ;
private:
	GLuint SkyboxTexture;

	// Inherited via Shader
	virtual void UpdateD3D11Uniforms(Transform * t, Camera * c, std::vector<Light*> lights) override;
};

