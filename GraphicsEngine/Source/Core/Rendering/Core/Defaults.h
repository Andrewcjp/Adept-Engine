#pragma once

class BaseTexture;
class Material;
class Asset_Shader;
class Defaults
{
public:
	static void Start();
	static void Shutdown();
	static BaseTexture* GetDefaultTexture();
	static Material* GetDefaultMaterial();

	static Asset_Shader * GetDefaultShaderAsset();

private:
	Asset_Shader* DefaultShaderMat;
	Defaults();
	~Defaults();
	static Defaults* Instance;
	BaseTexture* DefaultTexture;
};

