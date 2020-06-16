#include "ShaderProgramBase.h"
#include <set>
#include "DeviceContext.h"

ShaderProgramBase::ShaderProgramBase(DeviceContext* context)
{
	Context = context;
	AddDefaultDefines(Defines, context);
}
void ShaderProgramBase::AddDefaultDefines(std::vector<ShaderProgramBase::Shader_Define>& Defines, DeviceContext* context)
{
	if (!RHI::IsVulkan())
	{
		//the vulkan to HLSL Compiler defines this for us
		//Defines.push_back(Shader_Define("VULKAN", std::to_string(RHI::IsVulkan())));
	}
	Defines.push_back(Shader_Define("DX12", std::to_string(RHI::IsD3D12() || RHI::IsNullRHIActingDX12())));
	Defines.push_back(Shader_Define("FULL_UAV_LOAD", std::to_string(context->GetCaps().SupportTypedUAVLoads)));
	Defines.push_back(Shader_Define("HARDWARE_SUPPORT_SAMPLER_FEEDBACK", std::to_string(context->GetCaps().SamplerFeedbackMode == ESamplerFeedBackSupportMode::FullHardware)));
}
ShaderProgramBase::~ShaderProgramBase()
{

}

EShaderError::Type ShaderProgramBase::AttachAndCompileShaderFromFile(const char * filename, EShaderType::Type type)
{
	return AttachAndCompileShaderFromFile(filename, type, "main");
}

EShaderError::Type ShaderProgramBase::AttachAndCompileShaderFromSource(std::string SRC, EShaderType::Type type)
{
	return EShaderError::SHADER_ERROR_UNSUPPORTED;
}

void ShaderProgramBase::ModifyCompileEnviroment(Shader_Define Define)
{
	Defines.push_back(Define);
}

void ShaderProgramBase::NumberRS()
{
	for (int i = 0; i < GeneratedParams.size(); i++)
	{
		GeneratedParams[i].SignitureSlot = i;
	}
}

bool ShaderProgramBase::IsComputeShader()
{
	return IsCompute;
}

bool ShaderProgramBase::IsRSResolved() const
{
	return HasResolved;
}

void ShaderProgramBase::ResolveRS()
{
	//RemoveDupes();
	NumberRS();
	HasResolved = true;
}

glm::ivec3 ShaderProgramBase::GetComputeThreadSize() const
{
	return glm::ivec3(1);
}
