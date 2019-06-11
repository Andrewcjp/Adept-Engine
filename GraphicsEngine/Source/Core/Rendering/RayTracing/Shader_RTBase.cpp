#include "Shader_RTBase.h"


Shader_RTBase::Shader_RTBase(DeviceContext* C, std::string Name, ERTShaderType::Type Stage) :Shader(C)
{
	m_Shader->AttachAndCompileShaderFromFile(Name.c_str(), EShaderType::SHADER_RT_LIB);
	ShaderStage = Stage;
}


Shader_RTBase::~Shader_RTBase()
{

}

ERTShaderType::Type Shader_RTBase::GetStage()
{
	return ShaderStage;
}

void Shader_RTBase::AddExport(std::string Symbol)
{
	ExportedSymbols.push_back(Symbol);
}

std::vector<std::string>& Shader_RTBase::GetExports()
{
	return ExportedSymbols;
}
