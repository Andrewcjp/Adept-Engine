#include "Shader_RTBase.h"


Shader_RTBase::Shader_RTBase(DeviceContext* C, std::string Name, ERTShaderType::Type Stage) :Shader(C)
{
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("MAX_POINT_SHADOWS", std::to_string(std::max(RHI::GetRenderConstants()->MAX_DYNAMIC_POINT_SHADOWS, 1))));
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("MAX_DIR_SHADOWS", std::to_string(std::max(RHI::GetRenderConstants()->MAX_DYNAMIC_DIRECTIONAL_SHADOWS, 1))));
	m_Shader->ModifyCompileEnviroment(ShaderProgramBase::Shader_Define("MAX_LIGHTS", std::to_string(RHI::GetRenderConstants()->MAX_LIGHTS)));
#if RHI_SUPPORTS_RT
	m_Shader->AttachAndCompileShaderFromFile(Name.c_str(), EShaderType::SHADER_RT_LIB);
#endif
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

void Shader_RTBase::InitRS()
{
	LocalRootSig.SetRootSig(this->GetShaderParameters());
}

bool Shader_RTBase::IsShaderSupported(const ShaderComplieSettings& args)
{
	return args.RTSupported && args.ShaderModel == EShaderSupportModel::SM6;
}
