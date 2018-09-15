#pragma once
#include "RHI/Shader.h"
class Shader_Particle :public Shader
{
public:
	Shader_Particle(class DeviceContext* device);
	~Shader_Particle();
	void UpdateOGLUniforms(Transform * t, Camera * c, std::vector<Light*> lights) ;
private:
	int CameraRight_worldspace;
	int CameraUp_worldspace;
	int VP;

	// Inherited via Shader
	void UpdateD3D11Uniforms(Transform * t, Camera * c, std::vector<Light*> lights) ;
};

