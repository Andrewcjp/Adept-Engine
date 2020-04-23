#include "GlobalShaderLibrary.h"
#include "Shader_Pair.h"

Shader_Pair* GlobalShaderLibrary::BuildTopLevelVXShader = nullptr;
Shader_Pair* GlobalShaderLibrary::VolumeDownSample = nullptr;

void GlobalShaderLibrary::Init()
{
	Shader_Pair::IsPartOfGlobalShaderLibrary = true;
	BuildTopLevelVXShader = new Shader_Pair(RHI::GetDefaultDevice(), { "Voxel\\BuildTopLevelVoxel" }, { EShaderType::SHADER_COMPUTE });
	VolumeDownSample = new Shader_Pair(RHI::GetDefaultDevice(), { "Voxel\\VoxelMipCS" }, { EShaderType::SHADER_COMPUTE });

	Shader_Pair::IsPartOfGlobalShaderLibrary = false;
}
