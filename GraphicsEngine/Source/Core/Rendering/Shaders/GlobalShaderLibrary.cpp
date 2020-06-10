#include "GlobalShaderLibrary.h"
#include "Shader_Pair.h"
#include "CrossDeviceShaderPair.h"

CrossDeviceShaderPair* GlobalShaderLibrary::BuildTopLevelVXShader = nullptr;
CrossDeviceShaderPair* GlobalShaderLibrary::VolumeDownSample = nullptr;
CrossDeviceShaderPair* GlobalShaderLibrary::LightCullingShader = nullptr;
CrossDeviceShaderPair* GlobalShaderLibrary::TiledLightingApplyShader = nullptr;
CrossDeviceShaderPair* GlobalShaderLibrary::FixedVelocityShaders = nullptr;
CrossDeviceShaderPair* GlobalShaderLibrary::TestSamplerFeedbackShader = nullptr;

CrossDeviceShaderPair* GlobalShaderLibrary::TerrainShader = nullptr;

GlobalPermutation* GlobalShaderLibrary::VRXResolveShader = nullptr;


void GlobalShaderLibrary::Init()
{
	Shader_Pair::IsPartOfGlobalShaderLibrary = true;
	BuildTopLevelVXShader = new CrossDeviceShaderPair( { "Voxel\\BuildTopLevelVoxel" }, { EShaderType::SHADER_COMPUTE });
	VolumeDownSample = new CrossDeviceShaderPair( { "Voxel\\VoxelMipCS" }, { EShaderType::SHADER_COMPUTE });
	LightCullingShader = new CrossDeviceShaderPair( { "Culling\\LightCullCS" }, { EShaderType::SHADER_COMPUTE },
		{ ShaderProgramBase::Shader_Define("LIGHTCULLING_TILE_SIZE", std::to_string(RHI::GetRenderConstants()->LIGHTCULLING_TILE_SIZE)),ShaderProgramBase::Shader_Define("MAX_LIGHTS", std::to_string(RHI::GetRenderConstants()->MAX_LIGHTS)) });

	TiledLightingApplyShader = new CrossDeviceShaderPair( { "Shading\\TiledLightingApplyCS" }, { EShaderType::SHADER_COMPUTE },
		{ ShaderProgramBase::Shader_Define("LIGHTCULLING_TILE_SIZE", std::to_string(RHI::GetRenderConstants()->LIGHTCULLING_TILE_SIZE)),ShaderProgramBase::Shader_Define("MAX_LIGHTS", std::to_string(RHI::GetRenderConstants()->MAX_LIGHTS)) });
	FixedVelocityShaders = new CrossDeviceShaderPair( { "Velocity_vs", "VelocityWrite" }, { EShaderType::SHADER_VERTEX,EShaderType::SHADER_FRAGMENT });

	VRXResolveShader = new GlobalPermutation({ "VRX\\VRRResolve" }, { EShaderType::SHADER_COMPUTE });
	VRXResolveShader->AddDefineMod("VRS_TILE_SIZE", "8");
	VRXResolveShader->AddDefineMod("VRS_TILE_SIZE", "16");
	VRXResolveShader->SetDefine("VRS_TILE_SIZE", std::to_string(RHI::GetDefaultDevice()->GetCaps().VRSTileSize));

	TestSamplerFeedbackShader = new CrossDeviceShaderPair( { "Main_vs", "Tests\\SFSText_FS" }, { EShaderType::SHADER_VERTEX, EShaderType::SHADER_FRAGMENT });
	TerrainShader = new CrossDeviceShaderPair({ "Terrain\\Terrain_Vs","Terrain\\Terrain_PS_Def" }, { EShaderType::SHADER_VERTEX,EShaderType::SHADER_FRAGMENT });


	Shader_Pair::IsPartOfGlobalShaderLibrary = false;
}

void GlobalPermutation::AddDefineMod(std::string token, std::string value)
{
	std::vector<ShaderProgramBase::Shader_Define> ShaderDefines = m_Defines;
	ShaderDefines.push_back(ShaderProgramBase::Shader_Define(token, value));
	pairs.push_back(new CrossDeviceShaderPair(m_names, m_Stages, ShaderDefines));
	m_DefinePermutations.push_back(ShaderProgramBase::Shader_Define(token, value));
}

void GlobalPermutation::SetDefine(std::string token, std::string value)
{
	CurrentPermutation = GetPermutation(token, value);
}


Shader_Pair * GlobalPermutation::Get(DeviceContext* context)
{
	return CurrentPermutation->Get(context);
}

CrossDeviceShaderPair* GlobalPermutation::GetPermutation(std::string token, std::string value)
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
