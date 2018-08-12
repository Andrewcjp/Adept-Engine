#include "Stdafx.h"
#include "Asset_Shader.h"
#include "Editor/ShaderGraph/ShaderGraph.h"
#include "Rendering/Core/Material.h"
Asset_Shader::Asset_Shader(bool GenDefault)
{
	if (GenDefault)
	{
		Graph = new ShaderGraph("");
		Graph->CreateDefault();
		Graph->Complie();
	}
	else
	{
		Graph = new ShaderGraph("Test");
		Graph->test();
		Graph->Complie();
	}
}

Asset_Shader::~Asset_Shader()
{}

Material * Asset_Shader::GetMaterialInstance()
{
	Material::MaterialProperties Props = Material::MaterialProperties();
	Props.ShaderInUse = Graph->GetGeneratedShader();
	Props.TextureBinds = Graph->GetMaterialData();
	return new Material(Props);
}
