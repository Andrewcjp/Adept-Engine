#include "MaterialShader.h"
#include "Core/Assets/Asset_Shader.h"
#include "Rendering/ShaderGraph/ShaderGraph.h"



MaterialShader::MaterialShader(Asset_Shader* Shader)
{
	CurrentData.Shader = Shader;
}


MaterialShader::~MaterialShader()
{}

Shader_NodeGraph* MaterialShader::GetOrComplie(MaterialShaderComplieData&  data)
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
	Shader_NodeGraph* s = ShaderComplier::Get()->GetMaterialShader(data);
	ensure(s);
#if USEHASH
	ShaderPermutations.emplace(data.ToHash(), s);
#else
	ShaderPermutations.emplace(data.ToString(), s);
#endif
	return s;
}

void MaterialShader::SetShader(MaterialShaderComplieData& data)
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

TextureBindSet * MaterialShader::GetBinds()
{
	return CurrentData.Shader->GetGraph()->GetMaterialData();
}

ParmeterBindSet MaterialShader::GetParamBinds()
{
	return CurrentData.Shader->GetGraph()->GetParameters();
}

bool MaterialShaderComplieData::operator==(const MaterialShaderComplieData other) const
{
	//#Materals shader keyword
	return RenderPassUsage == other.RenderPassUsage && MaterialRenderType == other.MaterialRenderType;
}

bool MaterialShaderComplieData::operator<(const MaterialShaderComplieData & o) const
{
	return false;
}

std::string MaterialShaderComplieData::ToString()
{
	return Shader->GetName() + std::to_string(RenderPassUsage) + std::to_string(MaterialRenderType);
}

int MaterialShaderComplieData::ToHash()
{
	return std::hash<std::string>{} (ToString());
}