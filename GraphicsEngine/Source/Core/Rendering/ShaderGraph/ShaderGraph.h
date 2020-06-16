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

	FString& GetGraphName()
	{
		return GraphName;
	}
	void AddNodetoGraph(class ShaderGraphNode* Node);
	std::string GetCompliedCode();


	std::string Declares;
	int TReg = 20;
	int CurrentSlot = MainShaderRSBinds::Limit;

	void AddTexDecleration(std::string data, std::string name);
	bool IsPropertyDefined(std::string name);
	void AddDefine(std::string name);//type:
	void Compile();
	void BuildConstantBuffer();
	std::string GetMaterialConstantBufferCode();
//private:
	std::vector<std::string> DefinedVars;
	FString GraphName = "";
	std::vector<class ShaderGraphNode*> Nodes;
	class MasterNode* GraphMasterNode = nullptr;
	bool IsComplied = false;
	std::string CompliedCode;
	std::vector<PropertyLink*> BufferProps;
	std::string ConstantBufferCode = "";
	std::vector<std::string> Textures;
	ParmeterBindSet GetParameters();
};

