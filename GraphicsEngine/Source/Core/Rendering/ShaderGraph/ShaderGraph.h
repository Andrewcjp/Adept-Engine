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
	void SolidColour();
	void CreateDefault();
	
	bool Complie();
	class Shader* GetGeneratedShader();
	const Material::TextureBindSet* GetMaterialData();
	FString& GetGraphName() { return GraphName; }
	void AddNodetoGraph(class ShaderGraphNode* Node);
	void AddTexDecleration(std::string data, std::string name);
private:
	FString GraphName = "";
	std::vector<class ShaderGraphNode*> Nodes;
	class CoreProps* CoreGraphProperties = nullptr;
	Material::TextureBindSet* MaterialBinds = nullptr;
	std::string Declares;
	int TReg = 20;
	int CurrentSlot = 0;
	class Shader_NodeGraph* GeneratedShader = nullptr;
	std::string GetTemplateName();
};

