#include "Stdafx.h"
#include "ShaderGraph.h"
#include "ShaderGraphNode.h"
#include "Core/Assets/AssetManager.h"
#include <ios>
#include <iostream>
#include <fstream>
#include "Rendering/Shaders/Shader_NodeGraph.h"
#include "Core/Platform/PlatformCore.h"
ShaderGraph::ShaderGraph(FString Name)
{
	GraphName = Name;
	CoreGraphProperties = new CoreProps();
	MaterialBinds = new Material::TextureBindSet();
	CurrentSlot = MainShaderRSBinds::Limit + 1;
}

ShaderGraph::~ShaderGraph()
{}

void ShaderGraph::test()
{
	GraphName = "Test";
#if 0
	AddNodetoGraph(new SGN_Constant(CoreGraphProperties->Diffusecolour, glm::vec3(1, 1, 1)));
#else
	AddNodetoGraph(new SGN_Texture(CoreGraphProperties->Diffusecolour, "DiffuseMap"));
	AddNodetoGraph(new SGN_Texture(CoreGraphProperties->NormalDir, "NORMALMAP",TextureType::Normal));
#endif
}

void ShaderGraph::SolidColour()
{
	GraphName = "Test2";
#if 1
	AddNodetoGraph(new SGN_Constant(CoreGraphProperties->Diffusecolour, glm::vec3(1, 1, 1)));
#else
	AddNodetoGraph(new SGN_Texture(CoreGraphProperties->Diffusecolour, "DiffuseMap"));
	AddNodetoGraph(new SGN_Texture(CoreGraphProperties->NormalDir, "NORMALMAP", TextureType::Normal));
#endif
}

void ShaderGraph::CreateDefault()
{
	GraphName = "Default";
	AddNodetoGraph(new SGN_Texture(CoreGraphProperties->Diffusecolour, "DiffuseMap"));
}

bool WriteToFile(std::string filename, std::string data)
{
	std::string out;
	std::ofstream myfile(filename, std::ofstream::out);
	if (myfile.is_open())
	{
		std::string line;
		myfile.write(data.c_str(), data.length());
		myfile.close();
	}
	else
	{
		Log::OutS << "failed to save " << filename << Log::OutS;
		return false;
	}
	return true;
}

std::string ShaderGraph::GetTemplateName()
{
	if (RHI::GetRenderSettings()->IsDeferred)
	{
		return "MaterialTemplate_DEF_W_fs.hlsl";
	}
	return "MaterialTemplate_FWD_fs.hlsl";
}

bool ShaderGraph::Complie()
{
#if BUILD_SHIPPING
	//Temp For Default Materials
	for (int i = 0; i < Nodes.size(); i++)
	{
		Nodes[i]->GetComplieCode();
	}
	return true;
#endif
	std::string MainShader = AssetManager::instance->LoadFileWithInclude(GetTemplateName());
	ensure(!MainShader.empty());
	std::vector<std::string> split = StringUtils::Split(MainShader, '\n');
	const std::string TargetMarker = "//Insert Marker";
	const std::string TargetDefineMarker = "//Declares";
	std::string PreFile = "";
	std::string PostFile = "";
	std::string MidFile = "";
	bool Pre = true;
	int TargetMarkerindex = 0;
	int DeclareStartindex = 0;
	int postIndex = 0;
	for (int i = 0; i < split.size(); i++)
	{
		if (split[i].find(TargetDefineMarker) != -1)
		{
			DeclareStartindex = i;
		}
		if (split[i].find(TargetMarker) != -1)
		{
			TargetMarkerindex = i;
		}
	}
	for (int i = 0; i < split.size(); i++)
	{
		if (i < DeclareStartindex)
		{
			PreFile += split[i] + "\n";
		}
		if (i > DeclareStartindex && i < TargetMarkerindex)
		{
			MidFile += split[i] + "\n";
		}

		if (i > TargetMarkerindex)
		{
			PostFile += split[i] + "\n";
		}
	}

	std::string ComplieOutput;
	for (int i = 0; i < Nodes.size(); i++)
	{
		ComplieOutput += Nodes[i]->GetComplieCode();
	}

	PreFile += Declares + MidFile;

	std::string Path = AssetManager::GetShaderPath() + "Gen\\" + GraphName.ToSString() + ".hlsl";
	PlatformApplication::TryCreateDirectory(AssetManager::GetShaderPath() + "Gen");
	return WriteToFile(Path, PreFile + ComplieOutput + PostFile);
}

Shader* ShaderGraph::GetGeneratedShader()
{
	if (GeneratedShader == nullptr)
	{
		GeneratedShader = new Shader_NodeGraph(this);
	}
	return GeneratedShader;
}

const Material::TextureBindSet * ShaderGraph::GetMaterialData()
{
	return MaterialBinds;
}

void ShaderGraph::AddNodetoGraph(ShaderGraphNode * Node)
{
	Node->Root = this;
	Nodes.push_back(Node);
}

void ShaderGraph::AddTexDecleration(std::string data, std::string name)
{
	//data cotains Texture2D g_texture 
	//: register(t20);
	const std::string RegisterString = ": register(t" + std::to_string(TReg) + ");\n";
	Declares += data + RegisterString;
	MaterialBinds->AddBind(name, CurrentSlot, TReg);
	CurrentSlot++;
	TReg++;
}

