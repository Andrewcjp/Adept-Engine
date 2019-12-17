#pragma once
#include "RHI\Shader.h"
class Shader_FXAA: public Shader
{
public:
	Shader_FXAA(DeviceContext* con);
	~Shader_FXAA();
	DECLARE_GLOBAL_SHADER(Shader_FXAA);
};

