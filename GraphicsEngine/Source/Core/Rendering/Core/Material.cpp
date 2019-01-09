#include "Material.h"
#include <iostream>
#include "GPUStateCache.h"
#include "RHI/Shader.h"
#include "RHI/RHICommandList.h"
#include "Core/Platform/PlatformCore.h"
#include "Core/Assets/Asset_Shader.h"
#include "Core/Assets/ImageIO.h"
#include "Core/Assets/Archive.h"
#include "Core/Assets/SerialHelpers.h"
#include "Core/Assets/AssetManager.h"
Asset_Shader* Material::DefaultMaterial = nullptr;

Material::Material(BaseTexture * Diff, MaterialProperties props) :Material(props)
{
	UpdateBind("DiffuseMap", Diff);
}

Material::Material(MaterialProperties props)
{
	Properties = props;
	if (Properties.TextureBinds == nullptr)
	{
		CurrentBindSet = new TextureBindSet();
	}
	else
	{
		CurrentBindSet = new TextureBindSet(*Properties.TextureBinds);
	}
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
#if 1
	if (GetProperties()->ShaderInUse != nullptr)
	{
		list->SetPipelineStateObject_OLD(GetProperties()->ShaderInUse);
	}
	else
	{
		list->SetPipelineStateObject_OLD(Material::GetDefaultMaterialShader()/*, FrameBuffer*/);
	}
#endif
	for (auto const& Pair : CurrentBindSet->BindMap)
	{
		if (Pair.second.TextureObj == nullptr)
		{
			list->SetTexture(ImageIO::GetDefaultTexture(), Pair.second.RootSigSlot);
		}
		else
		{
			list->SetTexture(Pair.second.TextureObj, Pair.second.RootSigSlot);
		}
	}
}

void Material::UpdateBind(std::string Name, BaseTexture* NewTex)
{
	if (CurrentBindSet->BindMap.find(Name) != CurrentBindSet->BindMap.end())
	{
		if (CurrentBindSet->BindMap.at(Name).TextureObj != NewTex)
		{
			SafeRefRelease(CurrentBindSet->BindMap.at(Name).TextureObj);
			CurrentBindSet->BindMap.at(Name).TextureObj = NewTex;
			NewTex->AddRef();
		}
	}
	else
	{
		ensureMsgf(false, "Failed to Find Bind");
		//SafeRefRelease(NewTex);
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
	return GetTexturebind("NORMALMAP") != nullptr;
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

void SerialTextureBind(Archive * A, Material::TextureBindData* object)
{
	ArchiveProp(object->RegisterSlot);
	ArchiveProp(object->RootSigSlot);
	if (A->IsReading())
	{
		std::string Name;
		ArchiveProp_Alias(Name, object->TextureObj->TexturePath);
		object->TextureObj = AssetManager::DirectLoadTextureAsset(Name);
		object->TextureObj->AddRef();
	}
	else
	{
		ArchiveProp(object->TextureObj->TexturePath);
	}
}

void Material::ProcessSerialArchive(Archive * A)
{

	ArchiveProp(Properties.Metallic);
	ArchiveProp(Properties.Roughness);
	if (A->IsReading())
	{
		std::string ShaderName = "";
		ArchiveProp_Alias(ShaderName, Properties.ShaderInUse->GetName());
		//TEMP
		Asset_Shader* NewShader = nullptr;
		if (ShaderName == "Test")
		{
			NewShader = new Asset_Shader();
			NewShader->SetupTestMat();
		}
		else if (ShaderName == "Test2")
		{
			NewShader = new Asset_Shader();
			NewShader->SetupSingleColour();
		}
		else
		{
			Properties.ShaderInUse = Material::GetDefaultMaterialShader();
		}
		if (NewShader != nullptr)
		{
			NewShader->GetMaterialInstance(this);
			//CurrentBindSet->BindMap = Properties.TextureBinds;
		}
	}
	else
	{
		std::string tmp = "";
		if (Properties.ShaderInUse != nullptr)
		{
			tmp = Properties.ShaderInUse->GetName();
		}
		ArchiveProp_Alias(tmp, Properties.ShaderInUse->GetName());
	}
	if (A->IsReading())
	{
		CurrentBindSet->BindMap.clear();
	}
	A->LinkPropertyMap<std::string, Material::TextureBindData>(CurrentBindSet->BindMap, "CurrentBindSet->BindMap", &SerialTextureBind);

}

void Material::SetupDefaultBinding(TextureBindSet* TargetSet)
{
	TargetSet->BindMap.clear();
	TargetSet->BindMap.emplace("ALBEDOMAP", TextureBindData{ nullptr, ALBEDOMAP });
	TargetSet->BindMap.emplace("NORMALMAP", TextureBindData{ nullptr, NORMALMAP });
}

