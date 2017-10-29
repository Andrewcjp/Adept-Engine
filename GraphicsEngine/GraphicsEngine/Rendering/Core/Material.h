#pragma once
#include "RHI/BaseTexture.h"
//#include <GLEW\GL\glew.h>
class Material
{
public:
	Material(BaseTexture* Diff) {
		Diffusetexture = Diff;

	}
	~Material();
	BaseTexture* Diffusetexture;
	BaseTexture* Metallictex;
	BaseTexture* glosstex;
	BaseTexture* NormalMap;
	BaseTexture* DisplacementMap;
	void SetMaterialActive();
	void SetShadow(bool state) {
		DoesShadow = state;
	}
	bool GetDoesShadow() {
		return DoesShadow;
	}
private:
	bool DoesShadow = true;
};

