#include "Asset_Shader.h"
#include "Rendering/Core/Material.h"
#include "Rendering/ShaderGraph/ShaderGraph.h"
#include "../ShaderComplier.h"
#include "Rendering/Core/ShaderAssetInstance.h"

Asset_Shader::Asset_Shader(bool GenDefault)
{
	if (GenDefault)
	{
		CreateGenDefault();
		AssetPath = "shader.default";
	}
}

void Asset_Shader::CreateGenDefault()
{
	Graph = new ShaderGraph("");
	Graph->CreateDefault();
	Graph->Complie();
}

void Asset_Shader::SetupSingleColour()
{
	Graph = new ShaderGraph("Test2");
	Graph->SolidColour();
	Graph->Complie();
}

void Asset_Shader::SetupTestMat()
{
	Graph = new ShaderGraph("Colour");
	Graph->test();
	Graph->Complie();
}

Asset_Shader::~Asset_Shader()
{}

std::string & Asset_Shader::GetName()
{
	return Graph->GetGraphName().ToSString();
}

ShaderGraph * Asset_Shader::GetGraph()
{
	return Graph;
}

ShaderAssetInstance * Asset_Shader::CreateInstance()
{
	return nullptr;
}
