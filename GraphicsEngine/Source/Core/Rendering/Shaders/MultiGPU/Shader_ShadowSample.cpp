#include "Shader_ShadowSample.h"
#include "Rendering/Shaders/Shader_Main.h"
IMPLEMENT_GLOBAL_SHADER(Shader_ShadowSample);
DECLARE_GLOBAL_SHADER_PERMIUTATION(Shader_ShadowSample_1, Shader_ShadowSample, int, 1, nullptr);
DECLARE_GLOBAL_SHADER_PERMIUTATION(Shader_ShadowSample_2, Shader_ShadowSample, int, 2, nullptr);
DECLARE_GLOBAL_SHADER_PERMIUTATION(Shader_ShadowSample_3, Shader_ShadowSample, int, 3, nullptr);
DECLARE_GLOBAL_SHADER_PERMIUTATION(Shader_ShadowSample_4, Shader_ShadowSample, int, 4, nullptr);
Shader_ShadowSample::Shader_ShadowSample(DeviceContext * Context,int SampleCount) :Shader(Context)
{
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("MAX_SHADOW_SAMPLES", std::to_string(SampleCount)));
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("MAX_POINT_SHADOWS", std::to_string(std::max(RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS, 1))));
	m_Shader->AttachAndCompileShaderFromFile("Deferred_LightingPass_vs", EShaderType::SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("Shadow\\ShadowSample_fs", EShaderType::SHADER_FRAGMENT);
}

Shader_ShadowSample::~Shader_ShadowSample()
{}

std::vector<VertexElementDESC> Shader_ShadowSample::GetVertexFormat()
{
	std::vector<VertexElementDESC> out;
	out.push_back(VertexElementDESC{ "POSITION", 0, R32G32B32_FLOAT, 0, 0, EInputClassification::PER_VERTEX, 0 });
	out.push_back(VertexElementDESC{ "NORMAL", 0, R32G32B32_FLOAT, 0, 12, EInputClassification::PER_VERTEX, 0 });
	out.push_back(VertexElementDESC{ "TEXCOORD", 0, R32G32_FLOAT, 0, 24,EInputClassification::PER_VERTEX, 0 });
	out.push_back(VertexElementDESC{ "TANGENT", 0, R32G32B32_FLOAT, 0, 32,EInputClassification::PER_VERTEX, 0 });
	return out;
}
