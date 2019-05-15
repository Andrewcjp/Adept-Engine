
#include "Shader_NodeGraph.h"
#include "Rendering/ShaderGraph/ShaderGraph.h"
#include "../Core/Material.h"

//#Materals: refactor!
Shader_NodeGraph::Shader_NodeGraph(std::string Shadername, TextureBindSet* binds) :Shader_Main(true)
{
	m_Shader->AttachAndCompileShaderFromFile("Main_vs", EShaderType::SHADER_VERTEX);
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("TEST", "1"));
	m_Shader->AttachAndCompileShaderFromFile(Shadername.c_str(), EShaderType::SHADER_FRAGMENT);
	Binds = binds;
}

Shader_NodeGraph::~Shader_NodeGraph()
{

}

std::vector<Shader::VertexElementDESC> Shader_NodeGraph::GetVertexFormat()
{
	return Shader_Main::GetVertexFormat();
}

std::vector<ShaderParameter> Shader_NodeGraph::GetShaderParameters()
{
	std::vector<ShaderParameter> Params = Shader_Main::GetShaderParameters();
	std::map<std::string, TextureBindData>::const_iterator it;
	for (it = Binds->BindMap.begin(); it != Binds->BindMap.end(); it++)
	{
		Params.push_back(ShaderParameter(ShaderParamType::SRV, it->second.RootSigSlot, it->second.RegisterSlot));
	}
	return Params;
}

const std::string Shader_NodeGraph::GetName()
{
	return Matname;
}

TextureBindSet * Shader_NodeGraph::GetBinds()
{
	return Binds;
}
