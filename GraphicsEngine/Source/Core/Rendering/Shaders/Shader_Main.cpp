#include "Shader_Main.h"
#include "RHI/RHI.h"
#include "Rendering/Core/GPUStateCache.h"
#include "Core/GameObject.h"
#include "Core/Utils/MemoryUtils.h"

Shader_Main::Shader_Main(bool LoadForward)
{
	m_Shader = RHI::CreateShaderProgam();

	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("MAX_POINT_SHADOWS", std::to_string(std::max(RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS, 1))));
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("MAX_DIR_SHADOWS", std::to_string(std::max(RHI::GetRenderConstants()->MAX_DYNAMIC_DIRECTIONAL_SHADOWS, 1))));
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("MAX_LIGHTS", std::to_string(RHI::GetRenderConstants()->MAX_LIGHTS)));
}

Shader_Main::~Shader_Main()
{

}

std::vector<Shader::VertexElementDESC> Shader_Main::GetVertexFormat()
{
	return GetMainVertexFormat();
}

void Shader_Main::GetMainShaderSig(std::vector<Shader::ShaderParameter>& out)
{
	out.push_back(ShaderParameter(ShaderParamType::CBV, MainShaderRSBinds::GODataCBV, 0));
	out.push_back(ShaderParameter(ShaderParamType::CBV, MainShaderRSBinds::LightDataCBV, 1));
	out.push_back(ShaderParameter(ShaderParamType::CBV, MainShaderRSBinds::MVCBV, 2));
	//two shadows
	ShaderParameter parm = ShaderParameter(ShaderParamType::SRV, MainShaderRSBinds::DirShadow, 0, 1);
	parm.NumDescriptors = RHI::GetRenderConstants()->MAX_DYNAMIC_DIRECTIONAL_SHADOWS;
	out.push_back(parm);
	parm = ShaderParameter(ShaderParamType::SRV, MainShaderRSBinds::PointShadow, 1, 2);
	parm.NumDescriptors = RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS;
	out.push_back(parm);
	out.push_back(ShaderParameter(ShaderParamType::SRV, MainShaderRSBinds::DiffuseIr, 10));
	out.push_back(ShaderParameter(ShaderParamType::SRV, MainShaderRSBinds::SpecBlurMap, 11));
	out.push_back(ShaderParameter(ShaderParamType::SRV, MainShaderRSBinds::EnvBRDF, 12));
	out.push_back(ShaderParameter(ShaderParamType::SRV, MainShaderRSBinds::Limit, 13));
}

std::vector<Shader::VertexElementDESC> Shader_Main::GetMainVertexFormat()
{
	std::vector<Shader::VertexElementDESC> out;
	out.push_back(VertexElementDESC{ "POSITION", 0, FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	out.push_back(VertexElementDESC{ "NORMAL", 0, FORMAT_R32G32B32_FLOAT, 0, 12, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	out.push_back(VertexElementDESC{ "TEXCOORD", 0, FORMAT_R32G32_FLOAT, 0, 24,INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	out.push_back(VertexElementDESC{ "TANGENT", 0, FORMAT_R32G32B32_FLOAT, 0, 32,INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	return out;
}

std::vector<Shader::ShaderParameter> Shader_Main::GetShaderParameters()
{
	std::vector<Shader::ShaderParameter> Output;
	GetMainShaderSig(Output);
	return Output;
}

