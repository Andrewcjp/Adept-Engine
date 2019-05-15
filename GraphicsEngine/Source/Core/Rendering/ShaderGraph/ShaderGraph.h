#pragma once
#include "Core/EngineInc.h"
#include "Core/Assets/AssetTypes.h"
#include "Rendering/Core/Material.h"
#include "../Core/Mesh/MaterialShader.h"
#include "../Shaders/Shader_Main.h"

class PropertyLink;
class ShaderGraph
{
public:
	ShaderGraph(FString Name);
	~ShaderGraph();
	void test();
	void SolidColour();
	void CreateDefault();
	TextureBindSet* GetMaterialData();
	FString& GetGraphName()
	{
		return GraphName;
	}
	void AddNodetoGraph(class ShaderGraphNode* Node);
	std::string GetCompliedCode();


	TextureBindSet* MaterialBinds = nullptr;
	std::string Declares;
	int TReg = 20;
	int CurrentSlot = MainShaderRSBinds::Limit;

	void AddTexDecleration(std::string data, std::string name);
	bool IsPropertyDefined(std::string name);
	void AddDefine(std::string name);//type:
	void Complie();
	void BuildConstantBuffer();
	std::string GetMaterialConstantBufferCode();
//private:
	std::vector<std::string> DefinedVars;
	FString GraphName = "";
	std::vector<class ShaderGraphNode*> Nodes;
	std::string GetTemplateName(MaterialShaderComplieData& data);
	class MasterNode* GraphMasterNode = nullptr;
	bool IsComplied = false;
	std::string CompliedCode;
	std::vector<PropertyLink*> BufferProps;
	std::string ConstantBufferCode = "";

	ParmeterBindSet GetParameters();
};

