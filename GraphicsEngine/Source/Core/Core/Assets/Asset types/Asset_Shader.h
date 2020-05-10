#pragma once
#include "Core/Assets/AssetTypes.h"
#include "Rendering/Core/Material.h"
#include "BaseAsset.h"

class ShaderGraph;
class Asset_Shader : public BaseAsset
{
public:
	Asset_Shader(bool GenDefault = false);
	void CreateGenDefault();
	void SetupSingleColour();
	void SetupTestMat();
	~Asset_Shader();
	std::string & GetName();
	EMaterialRenderType::Type RenderType = EMaterialRenderType::Opaque;
	ShaderGraph* GetGraph();
private:
	std::string Name = "";
	ShaderGraph* Graph;
	AssetPathRef OutputFilePath;
};

