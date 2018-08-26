#include "Material.h"
#include <iostream>
#include "GPUStateCache.h"
#include "RHI/Shader.h"
#include "RHI/RHICommandList.h"
#include "Core/Platform/PlatformCore.h"
#include "Core/Assets/Asset_Shader.h"
Asset_Shader* Material::DefaultMaterial = nullptr;

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
	}
	else
	{
		CurrentBindSet = new TextureBindSet(*Properties.TextureBinds);
	}
	SetupBindings();
}

Material::~Material()
{
	std::map<std::string, Material::TextureBindData>::iterator it;
	for (it = CurrentBindSet->BindMap.begin(); it != CurrentBindSet->BindMap.end(); it++)
	{
		SafeRefRelease(it->second.TextureObj);
	}
}

void Material::SetMaterialActive(RHICommandList* list)
{
	for (auto const& Pair : CurrentBindSet->BindMap)
	{
		list->SetTexture(Pair.second.TextureObj, Pair.second.RootSigSlot);
	}
}

void Material::UpdateBind(std::string Name, BaseTexture* NewTex)
{
	if (CurrentBindSet->BindMap.find(Name) != CurrentBindSet->BindMap.end())
	{
		if (CurrentBindSet->BindMap.at(Name).TextureObj != NewTex)
		{
			CurrentBindSet->BindMap.at(Name).TextureObj->Release();
			CurrentBindSet->BindMap.at(Name).TextureObj = NewTex;
			NewTex->AddRef();
		}
	}
	else
	{
		//ensureMsgf(false, "Failed to Find Bind");
	}
}

BaseTexture * Material::GetTexturebind(std::string Name)
{
	if (CurrentBindSet->BindMap.find(Name) != CurrentBindSet->BindMap.end())
	{
		return CurrentBindSet->BindMap.at(Name).TextureObj;
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

Material::MaterialProperties* Material::GetProperties()
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
		UpdateBind("DiffuseMap", tex);
	}
}

bool Material::HasNormalMap()
{
	return GetTexturebind("NORMALMAP") != NullTexture2D;
}

void Material::SetupDefaultMaterial()
{
	if (DefaultMaterial == nullptr)
	{
		DefaultMaterial = new Asset_Shader(true);
	}
}

Material * Material::GetDefaultMaterial()
{
	return DefaultMaterial->GetMaterialInstance();
}

Shader * Material::GetDefaultMaterialShader()
{
	return DefaultMaterial->GetMaterialInstance()->GetProperties()->ShaderInUse;
}

void Material::SetupDefaultBinding(TextureBindSet* TargetSet)
{
	TargetSet->BindMap.clear();
	TargetSet->BindMap.emplace("ALBEDOMAP", TextureBindData{ NullTexture2D, ALBEDOMAP });
	TargetSet->BindMap.emplace("NORMALMAP", TextureBindData{ NullTexture2D, NORMALMAP });
}

void Material::SetupBindings()
{
	//init to zero binds
	std::map<std::string, Material::TextureBindData>::iterator it;
	for (it = CurrentBindSet->BindMap.begin(); it != CurrentBindSet->BindMap.end(); it++)
	{
		it->second.TextureObj = NullTexture2D;
	}
	for (int i = 0; i < CurrentBindSet->BindMap.size(); i++)
	{
		NullTexture2D->AddRef();
	}
}
