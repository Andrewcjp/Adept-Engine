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
	BaseTexture* Diffusetexture;
	BaseTexture* Metallictex;
	BaseTexture* glosstex;
	BaseTexture* NormalMap;
	BaseTexture* DisplacementMap;
	void SetShadow(bool state);
	bool GetDoesShadow();
	const MaterialProperties* GetProperties();
private:
	MaterialProperties Properties;
};

