#pragma once
#include "Core/EngineInc.h"
#include "Core/Assets/AssetTypes.h"
#include "Rendering/Core/Material.h"
class ShaderGraph
{
public:
	ShaderGraph(FString Name);
	~ShaderGraph();
	void test();
	bool Complie(AssetPathRef Outputfile);
	class Shader* GetGeneratedShader();
	Material::TextureBindSet* GetMaterialData();
	FString& GetGraphName() { return GraphName; }
private:
	FString GraphName = "";
	std::vector<class ShaderGraphNode*> Nodes;
	class CoreProps* CoreGraphProperties = nullptr;
	Material::TextureBindSet* MaterialBinds = nullptr;
};

