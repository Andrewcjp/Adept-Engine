#include "Shader_PreZ.h"
#include "Rendering\Shaders\Shader_Main.h"

IMPLEMENT_GLOBAL_SHADER(Shader_PreZ);
Shader_PreZ::Shader_PreZ(DeviceContext* d) :Shader(d)
{
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("MAX_INSTANCES", std::to_string(RHI::GetRenderConstants()->MAX_MESH_INSTANCES)));
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("WITH_INSTANCING", RHI::GetRenderSettings()->AllowMeshInstancing ? "1" : "0"));
	m_Shader->AttachAndCompileShaderFromFile("Main_vs", EShaderType::SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("WhiteOutput_fs", EShaderType::SHADER_FRAGMENT);
}

Shader_PreZ::~Shader_PreZ()
{}


std::vector<Shader::VertexElementDESC> Shader_PreZ::GetVertexFormat()
{
	return Shader_Main::GetMainVertexFormat();
}