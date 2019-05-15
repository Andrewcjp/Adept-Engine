#include "Stdafx.h"
#include "ShaderGraphComplier.h"
#include "ShaderGraph.h"
#include "Core\Utils\FileUtils.h"
#include "Core\Assets\AssetManager.h"
#include "..\Core\Mesh\MaterialTypes.h"
#include "Core\Assets\Asset_Shader.h"
#include "..\Shaders\Shader_NodeGraph.h"
#include "MasterNode.h"


ShaderGraphComplier::ShaderGraphComplier()
{}


ShaderGraphComplier::~ShaderGraphComplier()
{}

Shader_NodeGraph * ShaderGraphComplier::Complie(MaterialShaderComplieData & Data)
{
	std::string ShaderPath = "";
	if (!ComplieGraph(Data, ShaderPath))
	{
		return nullptr;
	}
	//#Materals Different binds per permutation?
	Shader_NodeGraph* Shader = new Shader_NodeGraph(ShaderPath, Data.Shader->GetGraph()->GetMaterialData());

	return Shader;
}
bool replace(std::string& str, const std::string& from, const std::string& to)
{
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}
bool ShaderGraphComplier::ComplieGraph(MaterialShaderComplieData& data, std::string &ShaderPath)
{
#if !WITH_EDITOR
	//Temp For Default Materials
	for (int i = 0; i < Nodes.size(); i++)
	{
		Nodes[i]->GetComplieCode();
	}
	return true;
#else
	ShaderGraph* Graph = data.Shader->GetGraph();
	if (!Graph->IsComplied)
	{
		Graph->Complie();
	}
	std::string MainShader = AssetManager::instance->LoadFileWithInclude(Graph->GraphMasterNode->GetTemplateName(data));
	ensure(!MainShader.empty());
	std::vector<std::string> split = StringUtils::Split(MainShader, '\n');
	const std::string TargetMarker = "//Insert Marker";
	const std::string TargetDefineMarker = "//Declares";
	const std::string TargetBuffer = "//BufferPoint";
	std::string PreFile = "";
	std::string PostFile = "";
	std::string MidFile = "";
	int TargetMarkerindex = 0;
	int DeclareStartindex = 0;
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

	std::string ComplieOutput = Graph->GetCompliedCode();

	PreFile += Graph->Declares + MidFile;
	std::string finaldata = PreFile + ComplieOutput + PostFile;
	replace(finaldata, TargetBuffer, Graph->GetMaterialConstantBufferCode());
	ShaderPath = "Gen\\" + Graph->GetGraphName().ToSString() + "_" + data.ToString();
	std::string Path = AssetManager::GetShaderPath() + ShaderPath + ".hlsl";
	PlatformApplication::TryCreateDirectory(AssetManager::GetShaderPath() + "Gen");
	return FileUtils::WriteToFile(Path, finaldata);
#endif
}