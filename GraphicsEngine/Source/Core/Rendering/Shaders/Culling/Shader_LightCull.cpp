#include "Shader_LightCull.h"
IMPLEMENT_GLOBAL_SHADER(Shader_LightCull);
Shader_LightCull::Shader_LightCull(DeviceContext * c) :Shader(c)
{
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("MAX_LIGHTS", std::to_string(RHI::GetRenderConstants()->MAX_LIGHTS)));
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("LIGHTCULLING_TILE_SIZE", std::to_string(RHI::GetRenderConstants()->LIGHTCULLING_TILE_SIZE)));
	m_Shader->AttachAndCompileShaderFromFile("Culling\\LightCullingCS", EShaderType::SHADER_COMPUTE);
}

Shader_LightCull::~Shader_LightCull()
{}
