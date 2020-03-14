#include "Shader_ReflectionRaygen.h"

IMPLEMENT_GLOBAL_SHADER_RT(Shader_ReflectionRaygen);

Shader_ReflectionRaygen::Shader_ReflectionRaygen(DeviceContext * D)
	:Shader_RTBase(D, "Raytracing\\ReflectionRayGenShader", ERTShaderType::RayGen)
{
	AddExport("rayGen");
	ChangeParamType("RayData", ShaderParamType::RootConstant);
	InitRS();
}

Shader_ReflectionRaygen::~Shader_ReflectionRaygen()
{}

