#include "Stdafx.h"
#include "Shader_NodeGraph.h"





Shader_NodeGraph::Shader_NodeGraph(bool LoadForward) :Shader_Main(LoadForward)
{}

Shader_NodeGraph::~Shader_NodeGraph()
{}

std::vector<Shader::VertexElementDESC> Shader_NodeGraph::GetVertexFormat()
{
	return 	Shader_Main::GetVertexFormat();
}

std::vector<Shader::ShaderParameter> Shader_NodeGraph::GetShaderParameters()
{
	std::vector<Shader::ShaderParameter> Params = Shader_Main::GetShaderParameters();

	return Params;
}
