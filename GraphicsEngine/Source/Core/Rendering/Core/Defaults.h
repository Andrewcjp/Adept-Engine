#pragma once
#include "RHI\BaseTexture.h"

class Material;
class Asset_Shader;
class Defaults
{
public:
	static void Start();
	static void Shutdown();
	static BaseTextureRef GetDefaultTexture();
	static Material* GetDefaultMaterial();

	static Asset_Shader * GetDefaultShaderAsset();

private:
	Asset_Shader* DefaultShaderMat;
	Defaults();
	~Defaults();
	static Defaults* Instance;
	BaseTextureRef DefaultTexture;
};

