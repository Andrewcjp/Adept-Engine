#include "Shader_RTMateralHit.h"

Shader_RTMateralHit::Shader_RTMateralHit(DeviceContext * C) :
	Shader_RTBase(C, "Raytracing\\MaterialBaseHitShader", ERTShaderType::Hit)
{
	InitRS();
	AddExport("chs");
}

Shader_RTMateralHit::~Shader_RTMateralHit()
{}

std::vector<ShaderParameter> Shader_RTMateralHit::GetShaderParameters()
{
	std::vector<ShaderParameter> out;
	out.push_back(ShaderParameter(ShaderParamType::SRV, 0, 1));
	out.push_back(ShaderParameter(ShaderParamType::SRV, 1, 2));
	out.push_back(ShaderParameter(ShaderParamType::SRV, 2, 3));
	return out;
}
