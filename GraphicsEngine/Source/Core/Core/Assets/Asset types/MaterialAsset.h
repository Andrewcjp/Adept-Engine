#pragma once
#include "BaseAsset.h"
#include "Core/Assets/AssetPtr.h"
#include "Rendering/Core/Mesh/MaterialTypes.h"


class TextureAsset;
class Asset_Shader;
class Material;
struct ParmeterBindSet;
struct MaterialParmeter
{
	AssetPtr<TextureAsset> TextureBind;
};
UCLASS()
class MaterialAsset : public BaseAsset
{
public:
	CLASS_BODY_Reflect();
	MaterialAsset();

	PROPERTY();
	AssetPtr<Asset_Shader> m_Shader;

	Material* CreateMaterial();
	void Sync(Material* mat);

	void SeralizeText(Archive* A) override;
	void SetupFromShader();
	ParmeterBindSet* GetAssetSet() { return &AssetSet; }
private:
	ParmeterBindSet AssetSet;
protected:
	void ProcessArchive(Archive* a) override;

};

