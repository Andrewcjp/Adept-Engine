#include "Shader.h"
#include "RHI.h"

Shader::Shader()
{}

Shader::Shader(DeviceContext * context)
{
	Device = context;
	m_Shader = RHI::CreateShaderProgam(context);
}

Shader::~Shader()
{
	SafeDelete(m_Shader);
}

ShaderProgramBase * Shader::GetShaderProgram()
{
	return m_Shader;
}

std::vector<ShaderParameter> Shader::GetShaderParameters()
{
	m_Shader->NumberRS();
	return m_Shader->GeneratedParams;
}

std::vector<Shader::VertexElementDESC> Shader::GetVertexFormat()
{
	//#SHADER reflect this!
	std::vector<Shader::VertexElementDESC> out;
	out.push_back(VertexElementDESC{ "POSITION", 0, FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	out.push_back(VertexElementDESC{ "NORMAL", 0, FORMAT_R32G32B32_FLOAT, 0, 12, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	out.push_back(VertexElementDESC{ "TEXCOORD", 0,FORMAT_R32G32_FLOAT, 0, 24,INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	return out;
}

bool Shader::IsComputeShader()
{
	return m_Shader->IsComputeShader();
}

void Shader::ApplyToCommandList(RHICommandList * list)
{}

int Shader::GetSlotForName(std::string name)
{
	for (int i = 0; i < m_Shader->GeneratedParams.size(); i++)
	{
		if (m_Shader->GeneratedParams[i].Name == name)
		{
			return m_Shader->GeneratedParams[i].SignitureSlot;
		}
	}
	LogEnsureMsgf(false,"failed to find name in shader");
	return -1;
}

const std::string Shader::GetName()
{
	return "_";
}