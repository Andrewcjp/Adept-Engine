#pragma once
#include "RHI/BaseTexture.h"
class Material
{
public:
	typedef std::pair<BaseTexture*, int> TextureBindpair;
	struct TextureBindSet
	{
		std::map<std::string, Material::TextureBindpair> BindMap;
	};
	struct MaterialProperties
	{
		bool UseMainShader = true;
		bool IsReflective = false;
		bool DoesShadow = true;
		float Roughness = 1.0f;
		float Metallic = 0.0f;
		class Shader* ShaderInUse = nullptr;
		TextureBindSet* TextureBinds = nullptr;
	};

	CORE_API Material(BaseTexture* Diff, MaterialProperties props = MaterialProperties());
	CORE_API Material(MaterialProperties props = MaterialProperties());
	~Material();
	void SetMaterialActive(class RHICommandList * list);

	void UpdateBind(std::string Name, BaseTexture * NewTex);
	BaseTexture* GetTexturebind(std::string Name);

	void SetShadow(bool state);
	bool GetDoesShadow();
	const MaterialProperties* GetProperties();
	void SetDisplacementMap(BaseTexture* tex);
	void SetNormalMap(BaseTexture * tex);
	void SetDiffusetexture(BaseTexture* tex);
	bool HasNormalMap();

private:
	TextureBindSet * CurrentBindSet = nullptr;
	void SetupDefaultBinding(TextureBindSet* TargetSet);
	void SetupBindings();
	MaterialProperties Properties;
	//bind to null
	BaseTexture* NullTexture2D = nullptr;
};

