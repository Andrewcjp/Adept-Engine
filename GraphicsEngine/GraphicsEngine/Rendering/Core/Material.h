#pragma once
#include "RHI/BaseTexture.h"
class Material
{
public:
	struct MaterialProperties
	{
		bool UseMainShader = true;
		bool IsReflective = false;
		bool DoesShadow = true;
	};
	Material(BaseTexture* Diff, MaterialProperties props = MaterialProperties());
	~Material();
	void SetMaterialActive(CommandListDef * list = nullptr);
	void SetMaterialActive(class RHICommandList * list);
	BaseTexture* Diffusetexture = nullptr;
	BaseTexture* Metallictex = nullptr;
	BaseTexture* glosstex = nullptr;
	BaseTexture* NormalMap = nullptr;
	BaseTexture* DisplacementMap = nullptr;
	void SetShadow(bool state);
	bool GetDoesShadow();
	const MaterialProperties* GetProperties();
private:
	MaterialProperties Properties;
};

