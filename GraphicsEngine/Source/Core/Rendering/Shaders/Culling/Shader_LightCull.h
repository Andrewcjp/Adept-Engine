#pragma once
#include "RHI\Shader.h"

class Shader_LightCull:public Shader
{
public:
	DECLARE_GLOBAL_SHADER(Shader_LightCull);
	Shader_LightCull(DeviceContext* c);
	~Shader_LightCull();
};

