#include "Shader_Main.h"
#include "RHI/RHI.h"
#include "Rendering/Core/GPUStateCache.h"
#include "Core/GameObject.h"
#include "Core/Utils/MemoryUtils.h"
#include <algorithm>
Shader_Main::Shader_Main(bool LoadForward)
{
	m_Shader = RHI::CreateShaderProgam();
	m_Shader->CreateShaderProgram();
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("MAX_POINT_SHADOWS", std::to_string(RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS)));
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("MAX_DIR_SHADOWS", std::to_string(RHI::GetRenderConstants()->MAX_DYNAMIC_DIRECTIONAL_SHADOWS)));
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("POINT_SHADOW_OFFSET", "t" + std::to_string(RHI::GetRenderConstants()->MAX_DYNAMIC_DIRECTIONAL_SHADOWS)));
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("MAX_LIGHTS", std::to_string(MAX_LIGHTS)));
//	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("WITH_DEFERRED", std::to_string((int)RHI::GetRenderSettings()->IsDeferred)));
#if 0
	m_Shader->AttachAndCompileShaderFromFile("Main_vs", SHADER_VERTEX);
	if (LoadForward)
	{
		m_Shader->AttachAndCompileShaderFromFile("Main_fs", SHADER_FRAGMENT);
	}
	else
	{
		m_Shader->AttachAndCompileShaderFromFile("DeferredWrite_fs", SHADER_FRAGMENT);
	}
#endif

	m_Shader->ActivateShaderProgram();

}
Shader_Main::~Shader_Main()
{
	//delete CLightBuffer;
	//delete CMVBuffer;
	//MemoryUtils::DeleteCArray(GameObjectTransformBuffer, MAX_DEVICE_COUNT);
}
std::vector<Shader::VertexElementDESC> Shader_Main::GetVertexFormat()
{
	std::vector<Shader::VertexElementDESC> out;
	out.push_back(VertexElementDESC{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	out.push_back(VertexElementDESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	out.push_back(VertexElementDESC{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	out.push_back(VertexElementDESC{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32,INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	return out;
}

void Shader_Main::SetNormalVis()
{
	if (vistate)
	{
		vistate = false;
	}
	else
	{
		vistate = true;
	}
}
void Shader_Main::SetFullBright()
{
	if (enabledFullBright)
	{
		enabledFullBright = false;
	}
	else
	{
		enabledFullBright = true;
	}
}
void Shader_Main::SetShadowVis()
{
	if (shadowvisstate)
	{
		shadowvisstate = false;
	}
	else
	{
		shadowvisstate = true;
	}
}

void Shader_Main::GetMainShaderSig(std::vector<Shader::ShaderParameter>& out)
{
	out.push_back(ShaderParameter(ShaderParamType::CBV, MainShaderRSBinds::GODataCBV, 0));
	out.push_back(ShaderParameter(ShaderParamType::CBV, MainShaderRSBinds::LightDataCBV, 1));
	out.push_back(ShaderParameter(ShaderParamType::CBV, MainShaderRSBinds::MVCBV, 2));
	//two shadows
	ShaderParameter parm = ShaderParameter(ShaderParamType::SRV, MainShaderRSBinds::DirShadow, 0);
	parm.NumDescriptors = RHI::GetRenderConstants()->MAX_DYNAMIC_DIRECTIONAL_SHADOWS;
	out.push_back(parm);
	parm = ShaderParameter(ShaderParamType::SRV, MainShaderRSBinds::PointShadow,/*(0 + )*/RHI::GetRenderConstants()->MAX_DYNAMIC_DIRECTIONAL_SHADOWS);
	parm.NumDescriptors = RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS;
	out.push_back(parm);
	out.push_back(ShaderParameter(ShaderParamType::SRV, MainShaderRSBinds::DiffuseIr, 10));
	out.push_back(ShaderParameter(ShaderParamType::SRV, MainShaderRSBinds::SpecBlurMap, 11));
	out.push_back(ShaderParameter(ShaderParamType::SRV, MainShaderRSBinds::EnvBRDF, 12));
	out.push_back(ShaderParameter(ShaderParamType::SRV, MainShaderRSBinds::Limit, 13));
}

std::vector<Shader::ShaderParameter> Shader_Main::GetShaderParameters()
{
	std::vector<Shader::ShaderParameter> Output;
	GetMainShaderSig(Output);
	return Output;
}

