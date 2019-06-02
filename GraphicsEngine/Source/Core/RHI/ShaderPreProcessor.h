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
};

