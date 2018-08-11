#include "Material.h"
#include <iostream>
#include "GPUStateCache.h"
#include "RHI/Shader.h"
#include "RHI/RHICommandList.h"
#include "Core/Platform/PlatformCore.h"
Material::Material(BaseTexture * Diff, MaterialProperties props) :Material(props)
{
	UpdateBind("ALBEDOMAP", Diff);
}

Material::Material(MaterialProperties props)
{
	Properties = props;
	NullTexture2D = RHI::CreateNullTexture();
	if (Properties.TextureBinds == nullptr)
	{
		CurrentBindSet = new TextureBindSet();
		SetupDefaultBinding(CurrentBindSet);
	}
	else
	{
		Properties.TextureBinds = Properties.TextureBinds;
	}
}

Material::~Material()
{
	std::map<std::string, Material::TextureBindpair>::iterator it;
	for (it = CurrentBindSet->BindMap.begin(); it != CurrentBindSet->BindMap.end(); it++)
	{
		SafeRefRelease(it->second.first);
	}
}

void Material::SetMaterialActive(RHICommandList* list)
{
	for (auto const& Pair : CurrentBindSet->BindMap)
	{
		list->SetTexture(Pair.second.first, Pair.second.second);
	}
}

void Material::UpdateBind(std::string Name, BaseTexture* NewTex)
{
	if (CurrentBindSet->BindMap.find(Name) != CurrentBindSet->BindMap.end())
	{
		if (CurrentBindSet->BindMap.at(Name).first != NewTex)
		{
			CurrentBindSet->BindMap.at(Name).first->Release();
			CurrentBindSet->BindMap.at(Name).first = NewTex;
			NewTex->AddRef();
		}
	}
	else
	{
		ensureMsgf(false, "Failed to Find Bind");
	}
}

BaseTexture * Material::GetTexturebind(std::string Name)
{
	if (CurrentBindSet->BindMap.find(Name) != CurrentBindSet->BindMap.end())
	{
		return CurrentBindSet->BindMap.at(Name).first;
	}
	return nullptr;
}

void Material::SetShadow(bool state)
{
	Properties.DoesShadow = state;
}

bool Material::GetDoesShadow()
{
	return Properties.DoesShadow;
}

const Material::MaterialProperties* Material::GetProperties()
{
	return &Properties;
}

void Material::SetDisplacementMap(BaseTexture * tex)
{
	if (tex != nullptr)
	{
		//UpdateBind("DisplacementMAP", tex);
	}
}

void Material::SetNormalMap(BaseTexture * tex)
{
	if (tex != nullptr)
	{
		UpdateBind("NORMALMAP", tex);
	}
}

void Material::SetDiffusetexture(BaseTexture * tex)
{
	if (tex != nullptr)
	{
		UpdateBind("ALBEDOMAP", tex);
	}
}

bool Material::HasNormalMap()
{
	return GetTexturebind("NORMALMAP") != NullTexture2D;
}

void Material::SetupDefaultBinding(TextureBindSet* TargetSet)
{
	TargetSet->BindMap.clear();
	TargetSet->BindMap.emplace("ALBEDOMAP", TextureBindpair(NullTexture2D, ALBEDOMAP));
	TargetSet->BindMap.emplace("NORMALMAP", TextureBindpair(NullTexture2D, NORMALMAP));
	for (int i = 0; i < TargetSet->BindMap.size(); i++)
	{
		NullTexture2D->AddRef();
	}
}

void Material::SetupBindings()
{
	for (auto Pair : CurrentBindSet->BindMap)
	{
		Pair.second.first->AddRef();
	}
}
