#pragma once
#include "ShaderProgramBase.h"


struct ShaderSourceFile;
//this is used to unify preprocessing over all shader compliers to avoid issues with ones that lack features.
//also handles API Overrides etc.
class ShaderPreProcessor
{
private:
	struct IncludeStack
	{
		int Limit = 0;
		std::string Relative;
		std::vector<std::string> IncludeList;
		bool HasSeenInclude(std::string include);
	};
public:
	static void ProcessForDef(std::string & data, ShaderProgramBase::Shader_Define * Def);
	RHI_API static bool PreProcessDefines(std::vector<ShaderProgramBase::Shader_Define> & defines, std::string & shaderData);
	static bool CheckIncludeExists(const std::string& file);
	RHI_API static std::string LoadShaderIncludeFile(std::string name, IncludeStack* Stack = nullptr);
	
	RHI_API static bool CheckCSOValid(std::string Name, const  std::string & ShaderNameHash);
	static void FindRootConstants(ShaderSourceFile* file);
private:
	static bool CompareCachedShaderBlobWithSRC(const std::string & ShaderName, const std::string & FullShaderName);
	//include Handler
	static const int	 includeLength = 9;
	static std::string ForceIncludeData;
	static std::string GetForceInclude();

};

