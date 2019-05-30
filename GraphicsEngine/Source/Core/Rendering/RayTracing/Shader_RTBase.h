#pragma once
#include "RHI/Shader.h"
class Shader_RTBase :public Shader
{
public:
	DECLARE_GLOBAL_SHADER(Shader_RTBase);
	Shader_RTBase(DeviceContext* C);
	~Shader_RTBase();
};

