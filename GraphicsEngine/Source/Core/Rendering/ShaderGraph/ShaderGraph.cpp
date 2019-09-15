
#include "ShaderGraph.h"
#include "ShaderGraphNode.h"
#include "Core/Assets/AssetManager.h"
#include <fstream>
#include "Rendering/Shaders/Shader_NodeGraph.h"
#include "Core/Platform/PlatformCore.h"
#include "Core/Utils/FileUtils.h"
#include "MasterNode.h"
#include "PBRMasterNode.h"
ShaderGraph::ShaderGraph(FString Name)
{
	GraphName = Name;
	GraphMasterNode = new PBRMasterNode();//new PBRMasterNode();
	MaterialBinds = new TextureBindSet();
}

ShaderGraph::~ShaderGraph()
{}

void ShaderGraph::test()
{
	GraphName = "Test";
#if 0
	AddNodetoGraph(new SGN_Constant(CoreGraphProperties->Diffusecolour, glm::vec3(1, 1, 1)));
#else
	AddNodetoGraph(new SGN_Texture(GraphMasterNode->Diffusecolour, "DiffuseMap"));
	AddNodetoGraph(new SGN_Texture(GraphMasterNode->NormalDir, "NORMALMAP", TextureType::Normal));
#endif
}

void ShaderGraph::SolidColour()
{
	GraphName = "Test2";
#if 1
	AddNodetoGraph(new SGN_Constant(GraphMasterNode->Diffusecolour, glm::vec3(1, 1, 1)));
#else
	AddNodetoGraph(new SGN_Texture(CoreGraphProperties->Diffusecolour, "DiffuseMap"));
	AddNodetoGraph(new SGN_Texture(CoreGraphProperties->NormalDir, "NORMALMAP", TextureType::Normal));
#endif
}

void ShaderGraph::CreateDefault()
{
	GraphName = "Default";
	AddNodetoGraph(new SGN_Texture(GraphMasterNode->Diffusecolour, "DiffuseMap"));
	GraphMasterNode->GetProp("Roughness")->ExposeToShader(this);
	GraphMasterNode->GetProp("Metallic")->ExposeToShader(this);
}

ParmeterBindSet ShaderGraph::GetParameters()
{
	ParmeterBindSet BindSet;
	for (int i = 0; i < BufferProps.size(); i++)
	{
		BindSet.AddParameter(BufferProps[i]->Name, BufferProps[i]->Type);
	}
	return BindSet;
}

TextureBindSet * ShaderGraph::GetMaterialData()
{
	ensure(IsComplied);
	return MaterialBinds;
}

void ShaderGraph::AddNodetoGraph(ShaderGraphNode * Node)
{
	Node->Root = this;
	Nodes.push_back(Node);
}

std::string ShaderGraph::GetCompliedCode()
{
	return CompliedCode;
}

void ShaderGraph::AddTexDecleration(std::string data, std::string name)
{
	//data contains Texture2D g_texture 
	//: register(t20);
	const std::string RegisterString = ": register(t" + std::to_string(TReg) + ");\n";
	Declares += data + RegisterString;
	MaterialBinds->AddBind(name, CurrentSlot, TReg);
	CurrentSlot++;
	TReg++;
}

bool ShaderGraph::IsPropertyDefined(std::string name)
{
	return VectorUtils::Contains(DefinedVars, name);
}

void ShaderGraph::AddDefine(std::string name)
{
	DefinedVars.push_back(name);
}

void ShaderGraph::Complie()
{
	BuildConstantBuffer();
	for (int i = 0; i < Nodes.size(); i++)
	{
		CompliedCode += Nodes[i]->GetComplieCode(this);
	}
	CompliedCode += GraphMasterNode->Complie(this);
	IsComplied = true;
}

void ShaderGraph::BuildConstantBuffer()
{
	std::string data = "struct MaterialParms{\n";
	int TargetSize = 256;
	int DataSize = 0;
	for (int i = 0; i < BufferProps.size(); i++)
	{
		data += BufferProps[i]->GetForBuffer() + "\n";
		DataSize += 4;//todo: other props
	}
	//each variable is padded by the hlsl complier to be 16 bytes long
	data += "float Pad[" + std::to_string((TargetSize - DataSize) / (4*4)) + "];\n";
	data += "};\n";

	data += "cbuffer MateralConstantBuffer : register(b3)\n{\n";
	data += "#if WITH_INSTANCING\n MaterialParms Parms[MAX_INSTANCES];\n#else \nMaterialParms Parms[1]; \n#endif \n";
	data += "};\n";
	ConstantBufferCode = data;
}

std::string ShaderGraph::GetMaterialConstantBufferCode()
{
	return ConstantBufferCode;
}


