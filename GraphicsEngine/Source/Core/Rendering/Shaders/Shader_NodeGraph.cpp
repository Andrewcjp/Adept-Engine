
#include "Shader_NodeGraph.h"
#include "Rendering/ShaderGraph/ShaderGraph.h"

//todo: refactor!
Shader_NodeGraph::Shader_NodeGraph(ShaderGraph* graph) :Shader_Main(true)
{
	if (RHI::GetRenderSettings()->IsDeferred)
	{
		m_Shader->AttachAndCompileShaderFromFile("Main_vs", EShaderType::SHADER_VERTEX);
	}
	else
	{
		m_Shader->AttachAndCompileShaderFromFile("Main_vs", EShaderType::SHADER_VERTEX);
	}
	std::string Data = "Gen\\" + graph->GetGraphName().ToSString();
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("TEST", "1"));
	m_Shader->AttachAndCompileShaderFromFile(Data.c_str(), EShaderType::SHADER_FRAGMENT);
	Matname = graph->GetGraphName().ToSString();
	Graph = graph;
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
	std::map<std::string, Material::TextureBindData>::const_iterator it;
	for (it = Graph->GetMaterialData()->BindMap.begin(); it != Graph->GetMaterialData()->BindMap.end(); it++)
	{
		Params.push_back(ShaderParameter(ShaderParamType::SRV, it->second.RootSigSlot, it->second.RegisterSlot));
	}
	return Params;
}

const std::string Shader_NodeGraph::GetName()
{
	return Matname;
}
