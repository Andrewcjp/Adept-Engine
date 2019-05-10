#include "Stdafx.h"
#include "Shader_PreZ.h"
#include "Rendering\Shaders\Shader_Main.h"

IMPLEMENT_GLOBAL_SHADER(Shader_PreZ);
Shader_PreZ::Shader_PreZ(DeviceContext* d) :Shader(d)
{
	m_Shader->AttachAndCompileShaderFromFile("Main_vs", EShaderType::SHADER_VERTEX);
}

Shader_PreZ::~Shader_PreZ()
{}

std::vector<ShaderParameter> Shader_PreZ::GetShaderParameters()
{
	std::vector<ShaderParameter> out;
	out.push_back(ShaderParameter(ShaderParamType::CBV, MainShaderRSBinds::GODataCBV, 0));
	out.push_back(ShaderParameter(ShaderParamType::CBV, MainShaderRSBinds::LightDataCBV, 1));
	out.push_back(ShaderParameter(ShaderParamType::CBV, MainShaderRSBinds::MVCBV, 2));
	return out;
}

std::vector<Shader::VertexElementDESC> Shader_PreZ::GetVertexFormat()
{
	return Shader_Main::GetMainVertexFormat();
}