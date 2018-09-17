#pragma once
#include "Core/Assets/AssetTypes.h"
class Asset_Shader
{
public:
	Asset_Shader(bool GenDefault = false);
	void SetupSingleColour();
	void SetupTestMat();
	~Asset_Shader();

	void Complie();
	bool IsGraph = false;
	class Material* GetMaterialInstance();
	void GetMaterialInstance(Material * mat);
	std::string & GetName();
private:
	std::string Name = "";
	class ShaderGraph* Graph;
	AssetPathRef OutputFilePath;
	void RegisterSelf();
	bool IsRegistered = false;
};

