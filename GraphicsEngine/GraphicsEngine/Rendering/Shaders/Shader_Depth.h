#pragma once
#include "../RHI/Shader.h"
#include <string>
#define CUBE_SIDES 6
#include "../Rendering/Core/FrameBuffer.h"
#include "glm\fwd.hpp"
class Shader_Depth :public Shader
{
public:
	Shader_Depth(bool LoadGeo);
	~Shader_Depth();
	void UpdateOGLUniforms(Transform* t, Camera* c, std::vector<Light*> lights);
	void SetShaderActive() override;
	std::vector<Shader::ShaderParameter> GetShaderParameters() override;
	bool LoadGeomShader = true;


private:
	float znear = 1;
	float zfar = 50; 
};

