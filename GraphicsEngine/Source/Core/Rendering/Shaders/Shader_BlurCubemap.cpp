#include "Shader_BlurCubemap.h"
#include "RHI\ShaderProgramBase.h"

IMPLEMENT_GLOBAL_SHADER(Shader_BlurCubemap);
Shader_BlurCubemap::Shader_BlurCubemap(DeviceContext * Dev) :Shader(Dev)
{
	m_Shader->AttachAndCompileShaderFromFile("BlurCubeMapFS", EShaderType::SHADER_COMPUTE);
	Init();
}

Shader_BlurCubemap::~Shader_BlurCubemap()
{}

void Shader_BlurCubemap::Init()
{
	ShaderData = RHI::CreateRHIBuffer(ERHIBufferType::Constant, Device);
	ShaderData->CreateConstantBuffer(sizeof(SData) * 6, 6);
	ShaderData->UpdateConstantBuffer(&Data[0], 0);
}

void Shader_BlurCubemap::Update()
{
	ShaderData->UpdateConstantBuffer(&Data[0], 0);
} 

std::vector<ShaderParameter> Shader_BlurCubemap::GetShaderParameters()
{
	if (m_Shader->GeneratedParams[0].Type == ShaderParamType::CBV)
	{
		m_Shader->GeneratedParams[0].Type = ShaderParamType::RootConstant;
	}
	return Shader::GetShaderParameters();

}
