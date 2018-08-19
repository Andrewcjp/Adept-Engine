#pragma once
#include "Core/Assets/AssetTypes.h"
class Asset_Shader
{
public:
	Asset_Shader(bool GenDefault = false);
	void SetupSingleColour();
	void SetupTestMat();
	~Asset_Shader();
	bool IsGraph = false;
	class Material* GetMaterialInstance();
private:

	class ShaderGraph* Graph;
	AssetPathRef OutputFilePath;
};

