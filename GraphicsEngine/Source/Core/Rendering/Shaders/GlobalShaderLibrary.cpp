#include "GlobalShaderLibrary.h"
#include "Shader_Pair.h"

Shader_Pair* GlobalShaderLibrary::BuildTopLevelVXShader = nullptr;
Shader_Pair* GlobalShaderLibrary::VolumeDownSample = nullptr;
Shader_Pair* GlobalShaderLibrary::LightCullingShader = nullptr;
Shader_Pair* GlobalShaderLibrary::TiledLightingApplyShader = nullptr;
Shader_Pair* GlobalShaderLibrary::FixedVelocityShaders = nullptr;

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
	Shader_Pair::IsPartOfGlobalShaderLibrary = false;
}
