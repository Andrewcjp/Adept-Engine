#include "Stdafx.h"
#include "Asset_Shader.h"
#include "Editor/ShaderGraph/ShaderGraph.h"
#include "Rendering/Core/Material.h"
Asset_Shader::Asset_Shader()
{
	Graph = new ShaderGraph("TEst");
	Graph->Complie(AssetPathRef("\\Gen\\Test.hlsl"));
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
