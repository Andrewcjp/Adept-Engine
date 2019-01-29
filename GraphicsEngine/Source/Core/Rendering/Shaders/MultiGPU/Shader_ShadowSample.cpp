#include "Shader_ShadowSample.h"
#include "Rendering/Shaders/Shader_Main.h"
IMPLEMENT_GLOBAL_SHADER(Shader_ShadowSample);
Shader_ShadowSample::Shader_ShadowSample(DeviceContext * Context) :Shader(Context)
{
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("MAX_SHADOW_SAMPLES", std::to_string(RHI::GetMGPUMode()->MAX_PRESAMPLED_SHADOWS)));
	m_Shader->AttachAndCompileShaderFromFile("ShadowSample_vs", EShaderType::SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("ShadowSample_fs", EShaderType::SHADER_FRAGMENT);
}

Shader_ShadowSample::~Shader_ShadowSample()
{}

std::vector<Shader::ShaderParameter> Shader_ShadowSample::GetShaderParameters()
{
	std::vector<Shader::ShaderParameter> Output;
	Output.push_back(ShaderParameter(ShaderParamType::CBV, 0, 0));
	Output.push_back(ShaderParameter(ShaderParamType::CBV, 1, 1));
	Output.push_back(ShaderParameter(ShaderParamType::CBV, 2, 2));
	Output.push_back(ShaderParameter(ShaderParamType::SRV, ShadowSRV, 0));
	Output.push_back(ShaderParameter(ShaderParamType::RootConstant, PreSampleCBV, 3));
	return Output;
}

std::vector<Shader::VertexElementDESC> Shader_ShadowSample::GetVertexFormat()
{
	std::vector<Shader::VertexElementDESC> out;
	out.push_back(VertexElementDESC{ "POSITION", 0, FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	out.push_back(VertexElementDESC{ "NORMAL", 0, FORMAT_R32G32B32_FLOAT, 0, 12, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	out.push_back(VertexElementDESC{ "TEXCOORD", 0, FORMAT_R32G32_FLOAT, 0, 24,INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	out.push_back(VertexElementDESC{ "TANGENT", 0, FORMAT_R32G32B32_FLOAT, 0, 32,INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	return out;
}
