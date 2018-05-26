#include "Material.h"
#include <iostream>
#include "GPUStateCache.h"
#include "../RHI/Shader.h"
#include "../RHI/RHICommandList.h"

Material::Material(BaseTexture * Diff, MaterialProperties props)
{
	Diffusetexture = Diff;
	Diff->AddRef();
	Properties = props;
	NullTexture2D = RHI::CreateNullTexture();
}

Material::~Material()
{
	if (Diffusetexture != nullptr)
	{
		SafeRelease(Diffusetexture);
	}
	if (NormalMap != nullptr)
	{
		SafeRelease(NormalMap);
	}
	if (DisplacementMap != nullptr)
	{
		SafeRelease(DisplacementMap);
	}
}


void Material::SetMaterialActive(RHICommandList* list)
{
	if (Diffusetexture != nullptr)
	{
		list->SetTexture(Diffusetexture, ALBEDOMAP);
	}
	if (NormalMap != nullptr)
	{
		list->SetTexture(NormalMap, NORMALMAP);
	}
	else
	{
		list->SetTexture(NullTexture2D, NORMALMAP);
	}
	if (DisplacementMap != nullptr)
	{
		//list->SetTexture(DisplacementMap, DISPMAP);
	}
	//Todo: Add Other Textures As Needed.
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
		tex->AddRef();
		DisplacementMap = tex;
	}
}

void Material::SetNormalMap(BaseTexture * tex)
{
	if (tex != nullptr)
	{
		tex->AddRef();
		NormalMap = tex;
	}
}

void Material::SetDiffusetexture(BaseTexture * tex)
{
	if (tex != nullptr)
	{
		tex->AddRef();
		Diffusetexture = tex;
	}
}

BaseTexture * Material::GetDisplacementMap()
{
	return DisplacementMap;
}

BaseTexture * Material::GetNormalMap()
{
	return NormalMap;
}

BaseTexture * Material::GetDiffusetexture()
{
	return Diffusetexture;
}
