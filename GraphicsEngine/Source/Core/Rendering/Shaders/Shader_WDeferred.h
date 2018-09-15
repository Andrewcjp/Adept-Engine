#pragma once
#include "RHI/Shader.h"
class Shader_WDeferred :public Shader
{
public:
	Shader_WDeferred(class DeviceContext* dev);
	~Shader_WDeferred();
};

