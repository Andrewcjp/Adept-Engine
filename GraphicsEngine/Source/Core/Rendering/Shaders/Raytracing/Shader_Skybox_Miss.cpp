#include "Shader_Skybox_Miss.h"
IMPLEMENT_GLOBAL_SHADER(Shader_Skybox_Miss);
Shader_Skybox_Miss::Shader_Skybox_Miss(DeviceContext * Con)
	:Shader_RTBase(Con, "Raytracing\\SkyboxMissShader", ERTShaderType::Miss)
{
	InitRS();
	AddExport("Miss");
}

Shader_Skybox_Miss::~Shader_Skybox_Miss()
{}

void Shader_Skybox_Miss::SetSkybox(BaseTextureRef T)
{
	LocalRootSig.SetTexture(0, T);
}

std::vector<ShaderParameter> Shader_Skybox_Miss::GetShaderParameters()
{
	std::vector<ShaderParameter> out;
	out.push_back(ShaderParameter(ShaderParamType::SRV, 0, 10));
	return out;
}
