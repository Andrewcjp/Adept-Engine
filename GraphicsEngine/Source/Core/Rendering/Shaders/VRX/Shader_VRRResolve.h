#pragma once
#include "RHI\Shader.h"

class DeviceContext;
class Shader_VRRResolve : public Shader
{
public:
	Shader_VRRResolve(DeviceContext* device);
	~Shader_VRRResolve();
	DECLARE_GLOBAL_SHADER(Shader_VRRResolve);
};

