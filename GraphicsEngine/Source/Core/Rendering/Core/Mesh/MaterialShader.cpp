#include "MaterialShader.h"
#include "Core/Assets/Asset types/Asset_Shader.h"
#include "Rendering/ShaderGraph/ShaderGraph.h"



MaterialShader::MaterialShader(Asset_Shader* Shader)
{
	CurrentData.Shader = Shader;
}


MaterialShader::~MaterialShader()
{}

Shader_NodeGraph* MaterialShader::GetOrComplie(MaterialShaderCompileData&  data)
{
#if USEHASH
	auto itro = ShaderPermutations.find(data.ToHash());
	if (itro != ShaderPermutations.end())
	{
		return itro->second;
	}
#else
	auto itro = ShaderPermutations.find(data.ToString());
	if (itro != ShaderPermutations.end())
	{
		return itro->second;
	}
#endif
	Shader_NodeGraph* s = ShaderCompiler::Get()->GetMaterialShader(data);
	ensure(s);
#if USEHASH
	ShaderPermutations.emplace(data.ToHash(), s);
#else
	ShaderPermutations.emplace(data.ToString(), s);
#endif
	return s;
}

void MaterialShader::SetShader(MaterialShaderCompileData& data)
{
	CurrentData = data;
	CurrentShader = GetOrComplie(data);
}

Shader_NodeGraph* MaterialShader::GetShader(EMaterialPassType::Type type)
{
	//todo:
	CurrentData.RenderPassUsage = type;
	return GetOrComplie(CurrentData);
}

ParmeterBindSet MaterialShader::GetParamBinds()
{
	return CurrentData.Shader->GetGraph()->GetParameters();
}

int MaterialShader::GetParamterSize()
{
	return GetParamBinds().GetSize();
}

bool MaterialShaderCompileData::operator==(const MaterialShaderCompileData other) const
{
	//#Materals shader keyword
	return RenderPassUsage == other.RenderPassUsage && MaterialRenderType == other.MaterialRenderType;
}

bool MaterialShaderCompileData::operator<(const MaterialShaderCompileData & o) const
{
	return false;
}

std::string MaterialShaderCompileData::ToString()
{
	std::string KeyWords = "";
	for (int i = 0; i < ShaderKeyWords.size(); i++)
	{
		KeyWords += ShaderKeyWords[i];
	}
	return Shader->GetName() + std::to_string(RenderPassUsage) + std::to_string(MaterialRenderType) + KeyWords;
}

int MaterialShaderCompileData::ToHash()
{
	return std::hash<std::string>{} (ToString());
}