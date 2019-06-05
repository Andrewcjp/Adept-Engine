#pragma once
#include "ShaderProgramBase.h"
//this is used to unify preprocessing over all shader compliers to avoid issues with ones that lack features.
//also handles API Overrides etc.
class ShaderPreProcessor
{
public:
	ShaderPreProcessor();
	~ShaderPreProcessor();
	static void ProcessForDef(std::string & data, ShaderProgramBase::Shader_Define * Def);
	RHI_API static bool PreProcessDefines(std::vector<ShaderProgramBase::Shader_Define> & defines, std::string & shaderData);
	RHI_API static std::string LoadShaderIncludeFile(std::string name, int limit, std::string Relative = std::string(), std::vector<std::string> * IncludeList = nullptr);
	
	RHI_API static bool CheckCSOValid(std::string Name, std::string ShaderNameHash);
private:
	static bool CompareCachedShaderBlobWithSRC(const std::string & ShaderName, const std::string & FullShaderName);
	//include Handler
	static const int	 includeLength = 9;
};

