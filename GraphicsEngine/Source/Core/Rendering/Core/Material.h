#pragma once
#include "RHI/BaseTexture.h"
class Material
{
public:
	struct TextureBindData
	{
		BaseTexture* TextureObj = nullptr;
		int RootSigSlot = 0;
		int RegisterSlot = 0;
	};
	struct TextureBindSet
	{
		std::map<std::string, Material::TextureBindData> BindMap;
		void AddBind(std::string name, int index, int Register)
		{
			BindMap.emplace(name, Material::TextureBindData{ nullptr, index ,Register });
		}
	};

	struct MaterialProperties
	{
		bool UseMainShader = true;
		bool IsReflective = false;
		bool DoesShadow = true;
		float Roughness = 1.0f;
		float Metallic = 0.0f;
		class Shader* ShaderInUse = nullptr;
		const TextureBindSet* TextureBinds = nullptr;
	};

	CORE_API Material(BaseTexture* Diff, MaterialProperties props = MaterialProperties());
	CORE_API Material(MaterialProperties props = MaterialProperties());
	~Material();
	void SetMaterialActive(class RHICommandList * list);

	void UpdateBind(std::string Name, BaseTexture * NewTex);
	BaseTexture* GetTexturebind(std::string Name);

	void SetShadow(bool state);
	bool GetDoesShadow();
	MaterialProperties* GetProperties();
	void SetDisplacementMap(BaseTexture* tex);
	void SetNormalMap(BaseTexture * tex);
	void SetDiffusetexture(BaseTexture* tex);
	bool HasNormalMap();
	static void SetupDefaultMaterial();
	static Material* GetDefaultMaterial();
	static Shader* GetDefaultMaterialShader();
private:
	TextureBindSet * CurrentBindSet = nullptr;
	void SetupDefaultBinding(TextureBindSet* TargetSet);
	void SetupBindings();
	MaterialProperties Properties;
	//bind to null
	static class Asset_Shader* DefaultMaterial;
};

