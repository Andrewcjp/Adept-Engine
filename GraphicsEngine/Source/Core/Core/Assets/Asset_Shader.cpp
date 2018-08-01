#include "Stdafx.h"
#include "Asset_Shader.h"
#include "Editor/ShaderGraph/ShaderGraph.h"

Asset_Shader::Asset_Shader()
{
	Graph = new ShaderGraph("TEst");
	Graph->Complie(AssetPathRef("\\Gen\\Test.hlsl"));
}


Asset_Shader::~Asset_Shader()
{}
