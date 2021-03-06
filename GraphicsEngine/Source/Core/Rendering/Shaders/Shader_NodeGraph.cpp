
#include "Shader_NodeGraph.h"
#include "Rendering/ShaderGraph/ShaderGraph.h"
#include "../Core/Material.h"
#include "../Core/VRXEngine.h"

//#Materals: refactor!
Shader_NodeGraph::Shader_NodeGraph(std::string Shadername) :Shader_Main(true)
{
	ShaderFilename = Shadername;
}

void Shader_NodeGraph::Init()
{
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("MAX_INSTANCES", std::to_string(RHI::GetRenderConstants()->MAX_MESH_INSTANCES)));
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("WITH_INSTANCING", RHI::GetRenderSettings()->AllowMeshInstancing ? "1" : "0"));

	VRXEngine::SetupVRRShader(this, Device);
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("TEST", "1"));
	m_Shader->AttachAndCompileShaderFromFile("Main_vs", EShaderType::SHADER_VERTEX);
	if (ShaderSource.length() == 0)
	{
		m_Shader->AttachAndCompileShaderFromFile(ShaderFilename.c_str(), EShaderType::SHADER_FRAGMENT);
	}
	else
	{
		m_Shader->AttachAndCompileShaderFromSource(ShaderSource, EShaderType::SHADER_FRAGMENT);
	}
	HasComplied = true;
}

Shader_NodeGraph::~Shader_NodeGraph()
{

}

std::vector<VertexElementDESC> Shader_NodeGraph::GetVertexFormat()
{
	return Shader_Main::GetVertexFormat();
}

std::vector<ShaderParameter> Shader_NodeGraph::GetShaderParameters()
{
#if 0
	std::vector<ShaderParameter> Params = Shader_Main::GetShaderParameters();
	std::map<std::string, TextureBindData>::const_iterator it;
	for (it = Binds->BindMap.begin(); it != Binds->BindMap.end(); it++)
	{
		Params.push_back(ShaderParameter(ShaderParamType::SRV, it->second.RootSigSlot, it->second.RegisterSlot));
	}
#else
	std::vector<ShaderParameter> Params = Shader::GetShaderParameters();
#endif
	return Params;
}

const std::string Shader_NodeGraph::GetName()
{
	return ShaderFilename;
}

bool Shader_NodeGraph::IsValid() const
{
	return HasComplied;
}

void Shader_NodeGraph::SetDefines(std::vector<std::string>& Define)
{
	for (int i = 0; i < Define.size(); i++)
	{
		m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define(Define[i], "1"));
	}
}
