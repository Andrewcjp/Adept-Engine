#pragma once
#include "RHI\Shader.h"

class Shader_ShowLightDensity :public Shader
{
public:
	DECLARE_GLOBAL_SHADER(Shader_ShowLightDensity);
	Shader_ShowLightDensity(DeviceContext* c);
	~Shader_ShowLightDensity();
};

