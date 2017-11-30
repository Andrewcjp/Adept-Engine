#include "Material.h"
#include <iostream>
#include "GPUStateCache.h"
#include "../RHI/Shader.h"


Material::~Material()
{
	if (Diffusetexture != nullptr)
	{
		Diffusetexture->FreeTexture();
	}	
	if (NormalMap != nullptr)
	{
		delete NormalMap;
	}
	if (DisplacementMap != nullptr)
	{
		delete DisplacementMap;
	}
}

void Material::SetMaterialActive(CommandListDef* list)
{
	
	if (Diffusetexture != nullptr)
	{
		if (list != nullptr)
		{
			Diffusetexture->Bind(list);
		}
		else
		{
			Diffusetexture->Bind(ALBEDOMAP);
		}
		
	}
	if (NormalMap != nullptr)
	{
		NormalMap->Bind(NORMALMAP);
	}
	if (DisplacementMap != nullptr)
	{
		DisplacementMap->Bind(DISPMAP);
	}
	//Todo: Add Other Textures As Needed.
}
