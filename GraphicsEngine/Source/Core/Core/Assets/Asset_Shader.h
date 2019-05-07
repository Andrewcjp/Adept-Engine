#pragma once
#include "Core/Assets/AssetTypes.h"
#include "Rendering/Core/Material.h"
class Asset_Shader
{
public:
	Asset_Shader(bool GenDefault = false);
	void CreateGenDefault();
	void SetupSingleColour();
	void SetupTestMat();
	~Asset_Shader();

	void Complie();
	bool IsGraph = false;
	class Material* GetMaterialInstance();
	Material* GetMaterial();
	void GetMaterialInstance(Material * mat);
	std::string & GetName();
	EMaterialRenderType::Type RenderType = EMaterialRenderType::Opaque;
private:
	std::string Name = "";
	class ShaderGraph* Graph;
	AssetPathRef OutputFilePath;
	void RegisterSelf();
	bool IsRegistered = false;
	Material* Instance = nullptr;
};

