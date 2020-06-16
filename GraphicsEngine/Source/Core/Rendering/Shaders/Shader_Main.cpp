#include "Shader_Main.h"
#include "RHI/RHI.h"
#include "Rendering/Core/GPUStateCache.h"
#include "Core/GameObject.h"
#include "Core/Utils/MemoryUtils.h"

Shader_Main::Shader_Main(bool LoadForward) :Shader(RHI::GetDefaultDevice())
{
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("MAX_POINT_SHADOWS", std::to_string(std::max(RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS, 1))));
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("MAX_DIR_SHADOWS", std::to_string(std::max(RHI::GetRenderConstants()->MAX_DYNAMIC_DIRECTIONAL_SHADOWS, 1))));
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("MAX_LIGHTS", std::to_string(RHI::GetRenderConstants()->MAX_LIGHTS)));
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("LIGHTCULLING_TILE_SIZE", std::to_string(RHI::GetRenderConstants()->LIGHTCULLING_TILE_SIZE)));
}

Shader_Main::~Shader_Main()
{

}

std::vector<VertexElementDESC> Shader_Main::GetVertexFormat()
{
	return GetMainVertexFormat();
}

void Shader_Main::GetMainShaderSig(std::vector<ShaderParameter>& out)
{
	out.push_back(ShaderParameter(ShaderParamType::CBV, MainShaderRSBinds::GODataCBV, 0));
	out.push_back(ShaderParameter(ShaderParamType::CBV, MainShaderRSBinds::LightDataCBV, 1));
	out.push_back(ShaderParameter(ShaderParamType::CBV, MainShaderRSBinds::MVCBV, 2));
	out.push_back(ShaderParameter(ShaderParamType::CBV, MainShaderRSBinds::MaterialData, 3));
	//two shadows
	ShaderParameter parm = ShaderParameter(ShaderParamType::SRV, MainShaderRSBinds::DirShadow, 0, 1);
	parm.NumDescriptors = RHI::GetRenderConstants()->MAX_DYNAMIC_DIRECTIONAL_SHADOWS;
	out.push_back(parm);
	parm = ShaderParameter(ShaderParamType::SRV, MainShaderRSBinds::PointShadow, 1, 2);
	parm.NumDescriptors = RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS;
	out.push_back(parm);
	out.push_back(ShaderParameter(ShaderParamType::SRV, MainShaderRSBinds::DiffuseIr, 10));

	parm = ShaderParameter(ShaderParamType::SRV, MainShaderRSBinds::SpecBlurMap, 11,3);
	parm.NumDescriptors = 1;
	out.push_back(parm);
	out.push_back(ShaderParameter(ShaderParamType::SRV, MainShaderRSBinds::EnvBRDF, 12));
	out.push_back(ShaderParameter(ShaderParamType::SRV, MainShaderRSBinds::PreSampledShadows, 13));
	out.push_back(ShaderParameter(ShaderParamType::RootConstant, MainShaderRSBinds::ResolutionCBV, 5));
	out[out.size() - 1].NumDescriptors = 2;
	out.push_back(ShaderParameter(ShaderParamType::Buffer, MainShaderRSBinds::LightBuffer, 31));
	out.push_back(ShaderParameter(ShaderParamType::Buffer, MainShaderRSBinds::LightDataBuffer, 30));
}

std::vector<VertexElementDESC> Shader_Main::GetMainVertexFormat()
{
	std::vector<VertexElementDESC> out;
	out.push_back(VertexElementDESC{ "POSITION", 0, R32G32B32_FLOAT, 0, 0, EInputClassification::PER_VERTEX, 0 });
	out[0].Stride = sizeof(OGLVertex);
	out.push_back(VertexElementDESC{ "NORMAL", 0, R32G32B32_FLOAT, 0, 12, EInputClassification::PER_VERTEX, 0 });
	out.push_back(VertexElementDESC{ "TEXCOORD", 0, R32G32_FLOAT, 0, 24,EInputClassification::PER_VERTEX, 0 });
	out.push_back(VertexElementDESC{ "TANGENT", 0, R32G32B32_FLOAT, 0, 32,EInputClassification::PER_VERTEX, 0 });
	return out;
}

std::vector<ShaderParameter> Shader_Main::GetShaderParameters()
{
	std::vector<ShaderParameter> Output;
	GetMainShaderSig(Output);
	return Output;
}

