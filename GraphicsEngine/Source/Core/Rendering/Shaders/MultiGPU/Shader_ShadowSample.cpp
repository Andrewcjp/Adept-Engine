#include "Shader_ShadowSample.h"
#include "Rendering/Shaders/Shader_Main.h"
IMPLEMENT_GLOBAL_SHADER(Shader_ShadowSample);
DECLARE_GLOBAL_SHADER_PERMIUTATION(Shader_ShadowSample_1, Shader_ShadowSample, int, 1);
DECLARE_GLOBAL_SHADER_PERMIUTATION(Shader_ShadowSample_2, Shader_ShadowSample, int, 2);
DECLARE_GLOBAL_SHADER_PERMIUTATION(Shader_ShadowSample_3, Shader_ShadowSample, int, 3);
DECLARE_GLOBAL_SHADER_PERMIUTATION(Shader_ShadowSample_4, Shader_ShadowSample, int, 4);
Shader_ShadowSample::Shader_ShadowSample(DeviceContext * Context,int SampleCount) :Shader(Context)
{
#if 0
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("MAX_SHADOW_SAMPLES", std::to_string(RHI::GetMGPUSettings()->MAX_PRESAMPLED_SHADOWS)));
#else
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("MAX_SHADOW_SAMPLES", std::to_string(SampleCount)));
#endif
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("MAX_POINT_SHADOWS", std::to_string(std::max(RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS, 1))));
	m_Shader->AttachAndCompileShaderFromFile("Shadow\\ShadowSample_vs", EShaderType::SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("Shadow\\ShadowSample_fs", EShaderType::SHADER_FRAGMENT);
}

Shader_ShadowSample::~Shader_ShadowSample()
{}

std::vector<Shader::ShaderParameter> Shader_ShadowSample::GetShaderParameters()
{
	std::vector<Shader::ShaderParameter> Output;
	Output.push_back(ShaderParameter(ShaderParamType::CBV, 0, 0));
	Output.push_back(ShaderParameter(ShaderParamType::CBV, 1, 1));
	Output.push_back(ShaderParameter(ShaderParamType::CBV, 2, 2));
	///Output.push_back(ShaderParameter(ShaderParamType::SRV, ShadowSRV, 0));
	ShaderParameter parm = ShaderParameter(ShaderParamType::SRV, ShadowSRV, 0, 2);
	parm.NumDescriptors = RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS;
	Output.push_back(parm);
	Output.push_back(ShaderParameter(ShaderParamType::RootConstant, PreSampleCBV, 3));
	Output[Output.size() - 1].NumDescriptors = 4;
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
