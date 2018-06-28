#pragma once
#include "RHI/Shader.h"
class Shader_WDeferred :public Shader
{
public:
	Shader_WDeferred();
	~Shader_WDeferred();
	void SetNormalState(bool hasnormalmap);
};

