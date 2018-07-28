#include "Stdafx.h"
#include "Shader_ShadowSample.h"
#include "../Shader_Main.h"

Shader_ShadowSample::Shader_ShadowSample(DeviceContext * Context)
{
	m_Shader = RHI::CreateShaderProgam(Context);
	m_Shader->AttachAndCompileShaderFromFile("ShadowSample_vs", SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("ShadowSample_fs", SHADER_FRAGMENT);
}


Shader_ShadowSample::~Shader_ShadowSample()
{

}

std::vector<Shader::ShaderParameter> Shader_ShadowSample::GetShaderParameters()
{
	std::vector<Shader::ShaderParameter> Output;
	Output.push_back(ShaderParameter(ShaderParamType::SRV, 0, 0));
	Output.push_back(ShaderParameter(ShaderParamType::CBV, 1, 0));
	Output.push_back(ShaderParameter(ShaderParamType::CBV, 2, 1));
	Output.push_back(ShaderParameter(ShaderParamType::CBV, 3, 2));
	return Output;
}

std::vector<Shader::VertexElementDESC> Shader_ShadowSample::GetVertexFormat()
{
	std::vector<Shader::VertexElementDESC> out;
	out.push_back(VertexElementDESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	out.push_back(VertexElementDESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	out.push_back(VertexElementDESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	out.push_back(VertexElementDESC{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32,INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	return out;
}
