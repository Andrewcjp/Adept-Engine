#include "Shader_ParticleDraw.h"
#include "RHI/RHI.h"
DECLARE_GLOBAL_SHADER(Shader_ParticleDraw)

Shader_ParticleDraw::Shader_ParticleDraw(DeviceContext* device) :Shader(device)
{
	m_Shader = RHI::CreateShaderProgam();
	m_Shader->AttachAndCompileShaderFromFile("Particles\\ParticleDraw", EShaderType::SHADER_VERTEX, "VSMain");
	m_Shader->AttachAndCompileShaderFromFile("Particles\\ParticleDraw", EShaderType::SHADER_FRAGMENT, "FSMain");
}

Shader_ParticleDraw::~Shader_ParticleDraw()
{}

std::vector<Shader::VertexElementDESC> Shader_ParticleDraw::GetVertexFormat()
{
	std::vector<Shader::VertexElementDESC> out;
	out.push_back(VertexElementDESC{ "POSITION", 0, FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	return out;
}

std::vector<Shader::ShaderParameter> Shader_ParticleDraw::GetShaderParameters()
{
	std::vector<Shader::ShaderParameter> Output;
	Output.push_back(ShaderParameter(ShaderParamType::RootConstant, 0, 0));
	Output.push_back(ShaderParameter(ShaderParamType::SRV, 1, 0, 0, RHI_SHADER_VISIBILITY::SHADER_VISIBILITY_ALL));
	Output.push_back(ShaderParameter(ShaderParamType::CBV, 2, 1));
	Output.push_back(ShaderParameter(ShaderParamType::SRV, 3, 1));
	return Output;
}