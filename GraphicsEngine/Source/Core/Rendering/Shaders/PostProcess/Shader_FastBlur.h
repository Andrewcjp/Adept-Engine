#pragma once
#include "RHI\Shader.h"
class Shader_FastBlur : public Shader
{
public:
	DECLARE_GLOBAL_SHADER(Shader_FastBlur);
	Shader_FastBlur(DeviceContext* cont);
	~Shader_FastBlur();
};

