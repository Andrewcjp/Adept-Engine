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
#include "RHI/RHI.h"
#include "../Shaders/Shader_Main.h"
#include "Defaults.h"
#include "Core/ObjectBase/SharedPtr.h"


void Material::UpdateShaderData()
{
	MaterialData->UpdateConstantBuffer(&ShaderProperties);
}

void Material::SetMaterialData(MaterialShaderData Data)
{
	ShaderProperties = Data;
	UpdateShaderData();
}

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
	MaterialData = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
	MaterialData->CreateConstantBuffer(sizeof(MaterialShaderData), 1);
}

Material::~Material()
{
	EnqueueSafeRHIRelease(MaterialData);
	SafeDelete(CurrentBindSet);
}

void Material::SetMaterialActive(RHICommandList* list)
{
	RHIPipeLineStateDesc desc;
	desc.DepthStencilState.DepthEnable = true;
	desc.DepthCompareFunction = COMPARISON_FUNC_LESS_EQUAL;
	if (RHI::GetRenderSettings()->UseZPrePass)
	{
		desc.DepthStencilState.DepthWrite = false;
	}
	if (MateralRenderType == EMaterialRenderType::Transparent)
	{
		desc.Blending = true;
		desc.Cull = false;
		desc.Mode = Full;
	}
	if (GetProperties()->ShaderInUse != nullptr)
	{
		desc.ShaderInUse = GetProperties()->ShaderInUse;
		list->SetPipelineStateDesc(desc);
	}
	else
	{
		desc.ShaderInUse = Material::GetDefaultMaterialShader();
		list->SetPipelineStateDesc(desc);
	}
	list->SetConstantBufferView(MaterialData, 0, MainShaderRSBinds::MaterialData);
	for (auto const& Pair : CurrentBindSet->BindMap)
	{
		if (Pair.second.TextureObj == nullptr)
		{
			list->SetTexture(ImageIO::GetDefaultTexture(), Pair.second.RootSigSlot);
		}
		else
		{
			list->SetTexture(Pair.second.TextureObj.Get(), Pair.second.RootSigSlot);
		}
	}
}

void Material::UpdateBind(std::string Name, BaseTextureRef NewTex)
{
	if (CurrentBindSet->BindMap.find(Name) != CurrentBindSet->BindMap.end())
	{
		if (CurrentBindSet->BindMap.at(Name).TextureObj != NewTex)
		{
			//SafeRefRelease(CurrentBindSet->BindMap.at(Name).TextureObj.Get());
			CurrentBindSet->BindMap.at(Name).TextureObj = NewTex;
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
		return CurrentBindSet->BindMap.at(Name).TextureObj.Get();
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

void Material::SetDiffusetexture(BaseTextureRef tex)
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

Material * Material::CreateDefaultMaterialInstance()
{
	return Defaults::GetDefaultShaderAsset()->GetMaterialInstance();
}

Material* Material::GetDefaultMaterial()
{
	return Defaults::GetDefaultShaderAsset()->GetMaterial();
}

Shader * Material::GetDefaultMaterialShader()
{
	return Defaults::GetDefaultShaderAsset()->GetMaterial()->GetProperties()->ShaderInUse;
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
	}
	else
	{
		ArchiveProp(object->TextureObj->TexturePath);
	}
}

void Material::ProcessSerialArchive(Archive * A)
{

	ArchiveProp(ShaderProperties.Metallic);
	ArchiveProp(ShaderProperties.Roughness);
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

EMaterialRenderType::Type Material::GetRenderPassType()
{
	return MateralRenderType;
}

void Material::SetupDefaultBinding(TextureBindSet* TargetSet)
{
	TargetSet->BindMap.clear();
	TargetSet->BindMap.emplace("ALBEDOMAP", TextureBindData{ nullptr, ALBEDOMAP });
	TargetSet->BindMap.emplace("NORMALMAP", TextureBindData{ nullptr, NORMALMAP });
}

