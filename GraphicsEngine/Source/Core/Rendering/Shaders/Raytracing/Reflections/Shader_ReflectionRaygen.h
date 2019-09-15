#pragma once
#include "Rendering/RayTracing/Shader_RTBase.h"

class Shader_ReflectionRaygen : public Shader_RTBase
{
public:
	DECLARE_GLOBAL_SHADER(Shader_ReflectionRaygen);
	RHI_API Shader_ReflectionRaygen(DeviceContext* D);
	~Shader_ReflectionRaygen();

	virtual std::vector<ShaderParameter> GetShaderParameters() override;

};

