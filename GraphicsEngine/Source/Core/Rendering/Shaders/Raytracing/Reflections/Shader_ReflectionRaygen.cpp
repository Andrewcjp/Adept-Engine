#include "Shader_ReflectionRaygen.h"

IMPLEMENT_GLOBAL_SHADER(Shader_ReflectionRaygen);
Shader_ReflectionRaygen::Shader_ReflectionRaygen(DeviceContext * D)
	:Shader_RTBase(D, "Raytracing\\ReflectionRayGenShader", ERTShaderType::RayGen)
{
	AddExport("rayGen");
}

Shader_ReflectionRaygen::~Shader_ReflectionRaygen()
{}

std::vector<ShaderParameter> Shader_ReflectionRaygen::GetShaderParameters()
{
	std::vector<ShaderParameter> out;
	//out.push_back(ShaderParameter(ShaderParamType::SRV, 0, 5));
	return out;
}
