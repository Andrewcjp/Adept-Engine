#pragma once
#include "RHI/ShaderProgramBase.h"
class Shader_Pair;
struct GlobalPermutation
{
	GlobalPermutation(const std::vector<std::string>& Names, const std::vector<EShaderType::Type>& StageList, const std::vector<ShaderProgramBase::Shader_Define>& Defines = std::vector<ShaderProgramBase::Shader_Define>())
	{
		m_names = Names;
		m_Stages = StageList;
		m_Defines = Defines;
	}
	void AddDefineMod(std::string token, std::string value);
	Shader_Pair* GetPermutation(std::string token, std::string value);
	void SetDefine(std::string token, std::string value);
	Shader_Pair* Get();
private:
	Shader_Pair* CurrentPermutation = nullptr;
	std::vector<std::string> m_names;
	std::vector<EShaderType::Type> m_Stages;
	std::vector<ShaderProgramBase::Shader_Define> m_Defines;
	std::vector<Shader_Pair*> pairs;
	std::vector<ShaderProgramBase::Shader_Define> m_DefinePermutations;
};
class GlobalShaderLibrary
{
public:
	static void Init();
	static Shader_Pair* BuildTopLevelVXShader;
	static Shader_Pair* VolumeDownSample;
	static Shader_Pair* LightCullingShader;
	static Shader_Pair* TiledLightingApplyShader;
	static Shader_Pair* FixedVelocityShaders;
	static GlobalPermutation* VRXResolveShader;
};

