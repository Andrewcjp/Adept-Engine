#pragma once
#include "../RHI/Shader.h"
class Shader_Grass :public Shader
{
public:
	Shader_Grass();
	~Shader_Grass();
	void UpdateOGLUniforms(Transform* t, Camera* c, std::vector<Light*> lights) override;

	// Inherited via Shader
	virtual void UpdateD3D11Uniforms(Transform * t, Camera * c, std::vector<Light*> lights) override;
	
private:
	
};

