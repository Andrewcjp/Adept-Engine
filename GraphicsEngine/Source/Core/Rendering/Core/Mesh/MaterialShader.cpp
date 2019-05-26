#include "Stdafx.h"
#include "MaterialShader.h"
#include "Core\Assets\Asset_Shader.h"
#include "..\..\ShaderGraph\ShaderGraph.h"


MaterialShader::MaterialShader(Asset_Shader* Shader)
{
	CurrentData.Shader = Shader;
}


MaterialShader::~MaterialShader()
{}

Shader_NodeGraph* MaterialShader::GetOrComplie(MaterialShaderComplieData& data)
{
#if 1
	auto itro = ShaderPermutations.find(data.ToString());
	if (itro != ShaderPermutations.end())
	{
		return itro->second;
	}
#else
	for (auto itor = ShaderPermutations.begin(); itor != ShaderPermutations.end(); itor++)
	{
		if (itor->first == data)
		{
			return itor->second;
		}
	}
#endif

	Shader_NodeGraph* s = ShaderComplier::Get()->GetMaterialShader(data);
	ensure(s);
	ShaderPermutations.emplace(data.ToString(), s);
	return s;
}

void MaterialShader::SetShader(MaterialShaderComplieData& data)
{
	CurrentData = data;
	CurrentShader = GetOrComplie(data);
}

Shader_NodeGraph* MaterialShader::GetShader(EMaterialPassType::Type type)
{
	if (CurrentData.RenderPassUsage == type)
	{
		return CurrentShader;
	}
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
