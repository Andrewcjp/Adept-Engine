#include "GlobalShaderLibrary.h"
#include "Shader_Pair.h"

Shader_Pair* GlobalShaderLibrary::BuildTopLevelVXShader = nullptr;
Shader_Pair* GlobalShaderLibrary::VolumeDownSample = nullptr;
Shader_Pair* GlobalShaderLibrary::LightCullingShader = nullptr;
Shader_Pair* GlobalShaderLibrary::TiledLightingApplyShader = nullptr;
Shader_Pair* GlobalShaderLibrary::FixedVelocityShaders = nullptr;

GlobalPermutation* GlobalShaderLibrary::VRXResolveShader = nullptr;

void GlobalShaderLibrary::Init()
{
	Shader_Pair::IsPartOfGlobalShaderLibrary = true;
	BuildTopLevelVXShader = new Shader_Pair(RHI::GetDefaultDevice(), { "Voxel\\BuildTopLevelVoxel" }, { EShaderType::SHADER_COMPUTE });
	VolumeDownSample = new Shader_Pair(RHI::GetDefaultDevice(), { "Voxel\\VoxelMipCS" }, { EShaderType::SHADER_COMPUTE });
	LightCullingShader = new Shader_Pair(RHI::GetDefaultDevice(), { "Culling\\LightCullCS" }, { EShaderType::SHADER_COMPUTE },
		{ ShaderProgramBase::Shader_Define("LIGHTCULLING_TILE_SIZE", std::to_string(RHI::GetRenderConstants()->LIGHTCULLING_TILE_SIZE)),ShaderProgramBase::Shader_Define("MAX_LIGHTS", std::to_string(RHI::GetRenderConstants()->MAX_LIGHTS)) });

	TiledLightingApplyShader = new Shader_Pair(RHI::GetDefaultDevice(), { "Shading\\TiledLightingApplyCS" }, { EShaderType::SHADER_COMPUTE },
		{ ShaderProgramBase::Shader_Define("LIGHTCULLING_TILE_SIZE", std::to_string(RHI::GetRenderConstants()->LIGHTCULLING_TILE_SIZE)),ShaderProgramBase::Shader_Define("MAX_LIGHTS", std::to_string(RHI::GetRenderConstants()->MAX_LIGHTS)) });
	FixedVelocityShaders = new Shader_Pair(RHI::GetDefaultDevice(), { "Velocity_vs", "VelocityWrite" }, { EShaderType::SHADER_VERTEX,EShaderType::SHADER_FRAGMENT });

	VRXResolveShader = new GlobalPermutation({ "VRX\\VRRResolve" }, { EShaderType::SHADER_COMPUTE });
	VRXResolveShader->AddDefineMod("VRS_TILE_SIZE", "8");
	VRXResolveShader->AddDefineMod("VRS_TILE_SIZE", "16");
	VRXResolveShader->SetDefine("VRS_TILE_SIZE", std::to_string(RHI::GetDefaultDevice()->GetCaps().VRSTileSize));
	Shader_Pair::IsPartOfGlobalShaderLibrary = false;
}

void GlobalPermutation::AddDefineMod(std::string token, std::string value)
{
	std::vector<ShaderProgramBase::Shader_Define> ShaderDefines = m_Defines;
	ShaderDefines.push_back(ShaderProgramBase::Shader_Define(token, value));
	pairs.push_back(new Shader_Pair(RHI::GetDefaultDevice(), m_names, m_Stages, ShaderDefines));
	m_DefinePermutations.push_back(ShaderProgramBase::Shader_Define(token, value));
}

void GlobalPermutation::SetDefine(std::string token, std::string value)
{
	CurrentPermutation = GetPermutation(token, value);
}

Shader_Pair * GlobalPermutation::Get()
{
	return CurrentPermutation;
}

Shader_Pair* GlobalPermutation::GetPermutation(std::string token, std::string value)
{
	for (int i = 0; i < m_DefinePermutations.size(); i++)
	{
		if (m_DefinePermutations[i].Name == token && m_DefinePermutations[i].Value == value)
		{
			return pairs[i];
		}
	}
	AD_ERROR("Failed to find shader permuation called %s of value %s", token.c_str(), value.c_str());
	return nullptr;
}
