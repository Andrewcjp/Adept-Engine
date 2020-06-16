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
	m_Shader->ResolveRS();
	CacheParms();
	return m_Shader->GeneratedParams;
}

std::vector<VertexElementDESC> Shader::GetVertexFormat()
{
	//#SHADER reflect this!
	std::vector<VertexElementDESC> out;;
	out.push_back(VertexElementDESC{ "POSITION", 0, R32G32B32_FLOAT, 0, 0, EInputClassification::PER_VERTEX, 0 });
	out.push_back(VertexElementDESC{ "NORMAL", 0, R32G32B32_FLOAT, 0, 12, EInputClassification::PER_VERTEX, 0 });
	out.push_back(VertexElementDESC{ "TEXCOORD", 0,R32G32_FLOAT, 0, 24,EInputClassification::PER_VERTEX, 0 });
	out[0].Stride = sizeof(OGLVertex);
	return out;
}

bool Shader::IsComputeShader()
{
	return m_Shader->IsComputeShader();
}

void Shader::ApplyToCommandList(RHICommandList * list)
{}

ShaderParameter * Shader::FindParam(const std::string & name)
{
	for (int i = 0; i < m_Shader->GeneratedParams.size(); i++)
	{
		if (m_Shader->GeneratedParams[i].Name == name)
		{
			return &m_Shader->GeneratedParams[i];
		}
	}
	return nullptr;
}

bool Shader::ChangeParamType(const std::string & name, ShaderParamType::Type type)
{
	ShaderParameter* P = FindParam(name);
	if (P != nullptr)
	{
		LogEnsureMsgf(P->Type != type, "Change To same type");
		P->Type = type;
		return true;
	}
	return false;
}

int Shader::GetSlotForName(const std::string & name)
{
	//Mixing is unsupported
	ensureMsgf(m_Shader->IsRSResolved(), "Get slot by name is not valid as the Root signiture has not been resolved ");
	for (int i = 0; i < m_Shader->GeneratedParams.size(); i++)
	{
		if (m_Shader->GeneratedParams[i].Name == name)
		{
			return m_Shader->GeneratedParams[i].SignitureSlot;
		}
	}
#if _DEBUG
	AD_ERROR("Failed to find variable '%s' in shader",name.c_str());
#endif
	return -1;
}

uint64 Shader::GetNameHash()
{
	if (Hash == 0)
	{
		Hash = std::hash<std::string>{} (GetName());
	}
	return Hash;
}

bool Shader::IsShaderSupported_SM6(const ShaderCompileSettings & args)
{
	return args.ShaderModel == EShaderSupportModel::SM6;
}

void Shader::CacheParms()
{}


const std::string Shader::GetName()
{
	return "_";
}