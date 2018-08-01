#include "Stdafx.h"
#include "ShaderGraph.h"
#include "ShaderGraphNode.h"
#include "Core/Assets/AssetManager.h"
#include <ios>
#include <iostream>
#include <fstream>
ShaderGraph::ShaderGraph(FString Name)
{
	GraphName = Name;
	CoreGraphProperties = new CoreProps();
	test();
}

ShaderGraph::~ShaderGraph()
{}

void ShaderGraph::test()
{
	Nodes.push_back(new SGN_Constant(CoreGraphProperties->Diffusecolour, glm::vec3(0, 0, 0)));
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
bool ShaderGraph::Complie(AssetPathRef Outputfile)
{
	std::string MainShader = AssetManager::instance->LoadFileWithInclude("Main_fs.hlsl");
	std::vector<std::string> split = StringUtils::Split(MainShader, '\n');
	const std::string TargetLine = "//Insert Marker";
	std::string PreFile = "";
	std::string PostFile = "";
	bool Pre = true;
	for (int i = 0; i < split.size(); i++)
	{
		if (Pre)
		{
			PreFile += split[i] + "\n";
		}
		else
		{
			PostFile += split[i] + "\n";
		}
		if (split[i].find(TargetLine) != -1)
		{
			Pre = false;
		}
	}

	std::string ComplieOutput;
	for (int i = 0; i < Nodes.size(); i++)
	{
		ComplieOutput += Nodes[i]->GetComplieCode();
	}
	return WriteToFile(Outputfile.GetFullPathToAsset(), PreFile + ComplieOutput + PostFile);
}

