#include "Stdafx.h"
#include "Shader_NodeGraph.h"
#include "Editor/ShaderGraph/ShaderGraph.h"
//todo: refactor!
Shader_NodeGraph::Shader_NodeGraph(ShaderGraph* graph) :Shader_Main(true)
{
	std::string Data = "Gen\\" + graph->GetGraphName().ToSString();
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("TEST","1"));
	m_Shader->AttachAndCompileShaderFromFile(Data.c_str(), SHADER_FRAGMENT);
	Matname = graph->GetGraphName().ToSString();
}

Shader_NodeGraph::~Shader_NodeGraph()
{

}

std::vector<Shader::VertexElementDESC> Shader_NodeGraph::GetVertexFormat()
{
	return Shader_Main::GetVertexFormat();
}

std::vector<Shader::ShaderParameter> Shader_NodeGraph::GetShaderParameters()
{
	std::vector<Shader::ShaderParameter> Params = Shader_Main::GetShaderParameters();

	return Params;
}

const std::string Shader_NodeGraph::GetName()
{
	return Matname;
}
