
#include "Shader_Line.h"
#include "Core/Assets/ShaderComplier.h"
#include "RHI/RHICommandList.h"

IMPLEMENT_GLOBAL_SHADER(Shader_Line);
DECLARE_GLOBAL_SHADER_PERMIUTATION(Shader_Line_2D_ON, Shader_Line, bool, true,nullptr);
DECLARE_GLOBAL_SHADER_PERMIUTATION(Shader_Line_2D_OFF, Shader_Line, bool, false, nullptr);
Shader_Line::Shader_Line(DeviceContext* device, bool is2D) : Shader(device)
{
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("TWOD_ONLY", is2D ? "1" : "0"));

	m_Shader->AttachAndCompileShaderFromFile("Debug\\debugline_vs", EShaderType::SHADER_VERTEX);
	m_Shader->AttachAndCompileShaderFromFile("Debug\\debugline_fs", EShaderType::SHADER_FRAGMENT);
	IsTwo = is2D;
}

Shader_Line::~Shader_Line()
{}

std::vector<VertexElementDESC> Shader_Line::GetVertexFormat()
{
	std::vector<VertexElementDESC> out;
	out.push_back(VertexElementDESC{ "POSITION", 0, FORMAT_R32G32B32_FLOAT, 0, 0, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	out.push_back(VertexElementDESC{ "NORMAL", 0, FORMAT_R32G32B32_FLOAT, 0, 12, INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	return out;
}

const std::string Shader_Line::GetName()
{
	if (IsTwo)
	{
		return "Shader_Line_2";
	}
	return "Shader_Line_3";
}

void Shader_Line::SetParameters(RHICommandList* List, RHIBuffer* UBO)
{
	List->SetConstantBufferView(UBO, 0, 0);
}