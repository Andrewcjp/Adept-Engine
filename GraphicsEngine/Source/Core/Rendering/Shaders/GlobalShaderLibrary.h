#pragma once
#include "RHI/ShaderProgramBase.h"
#include "CrossDeviceShaderPair.h"
class Shader_Pair;
class CrossDeviceShaderPair;
struct GlobalPermutation
{
	GlobalPermutation(const std::vector<std::string>& Names, const std::vector<EShaderType::Type>& StageList, const std::vector<ShaderProgramBase::Shader_Define>& Defines = std::vector<ShaderProgramBase::Shader_Define>())
	{
		m_names = Names;
		m_Stages = StageList;
		m_Defines = Defines;
	}
	void AddDefineMod(std::string token, std::string value);
	Shader_Pair * Get(DeviceContext * context);
	CrossDeviceShaderPair* GetPermutation(std::string token, std::string value);
	void SetDefine(std::string token, std::string value);
private:
	CrossDeviceShaderPair* CurrentPermutation = nullptr;
	std::vector<std::string> m_names;
	std::vector<EShaderType::Type> m_Stages;
	std::vector<ShaderProgramBase::Shader_Define> m_Defines;
	std::vector<CrossDeviceShaderPair*> pairs;
	std::vector<ShaderProgramBase::Shader_Define> m_DefinePermutations;
};
class GlobalShaderLibrary
{
public:
	static void Init();
	static CrossDeviceShaderPair* BuildTopLevelVXShader;
	static CrossDeviceShaderPair* VolumeDownSample;
	static CrossDeviceShaderPair* LightCullingShader;
	static CrossDeviceShaderPair* TiledLightingApplyShader;
	static CrossDeviceShaderPair* FixedVelocityShaders;
	static GlobalPermutation*	  VRXResolveShader;
	static CrossDeviceShaderPair* TestSamplerFeedbackShader;
	static CrossDeviceShaderPair*			 TerrainShader ;
};

