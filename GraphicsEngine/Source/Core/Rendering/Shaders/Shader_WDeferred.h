#pragma once
#include "RHI/Shader.h"
class Shader_WDeferred :public Shader
{
public:
	DECLARE_GLOBAL_SHADER(Shader_WDeferred);
	Shader_WDeferred(class DeviceContext* dev);
	~Shader_WDeferred();
};

