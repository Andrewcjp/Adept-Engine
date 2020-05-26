#include "Shader_RTMaterialHit.h"

Shader_RTMaterialHit::Shader_RTMaterialHit(DeviceContext * C) :
	Shader_RTBase(C, "Raytracing\\MaterialBaseHitShader", ERTShaderType::Hit)
{
	InitRS();
	AddExport("chs");
}

Shader_RTMaterialHit::~Shader_RTMaterialHit()
{}

std::vector<ShaderParameter> Shader_RTMaterialHit::GetShaderParameters()
{
	std::vector<ShaderParameter> out;
	out.push_back(ShaderParameter(ShaderParamType::SRV, 0, 1));
	out.push_back(ShaderParameter(ShaderParamType::SRV, 1, 2));
	out.push_back(ShaderParameter(ShaderParamType::SRV, 2, 3));
	return out;
}
