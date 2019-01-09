
#include "Shader_ParticleCompute.h"
IMPLEMENT_GLOBAL_SHADER(Shader_ParticleCompute);
IMPLEMENT_GLOBAL_SHADER(Shader_StartSimulation);
IMPLEMENT_GLOBAL_SHADER(Shader_EndSimulation);
IMPLEMENT_GLOBAL_SHADER(Shader_ParticleEmit);
Shader_ParticleCompute::Shader_ParticleCompute(DeviceContext* dev) :Shader(dev)
{
	m_Shader = RHI::CreateShaderProgam();
	m_Shader->AttachAndCompileShaderFromFile("Particles\\ParticleSimulate_CS", EShaderType::SHADER_COMPUTE);
}

Shader_ParticleCompute::~Shader_ParticleCompute()
{}

std::vector<Shader::ShaderParameter> Shader_ParticleCompute::GetShaderParameters()
{
	std::vector<Shader::ShaderParameter> Output;
	Output.push_back(ShaderParameter(ShaderParamType::UAV, 0, 0));
	Output.push_back(ShaderParameter(ShaderParamType::UAV, 1, 1));
	Output.push_back(ShaderParameter(ShaderParamType::SRV, 2, 2));
	Output.push_back(ShaderParameter(ShaderParamType::UAV, 3, 2));
	Output.push_back(ShaderParameter(ShaderParamType::UAV, 4, 3));
	return Output;
}

Shader_StartSimulation::Shader_StartSimulation(DeviceContext* dev) :Shader(dev)
{
	m_Shader = RHI::CreateShaderProgam();
	m_Shader->AttachAndCompileShaderFromFile("Particles\\StartUpdate_CS", EShaderType::SHADER_COMPUTE);
}

Shader_StartSimulation::~Shader_StartSimulation()
{}

std::vector<Shader::ShaderParameter> Shader_StartSimulation::GetShaderParameters()
{
	std::vector<Shader::ShaderParameter> Output;
	Output.push_back(ShaderParameter(ShaderParamType::UAV, 0, 0));
	Output.push_back(ShaderParameter(ShaderParamType::UAV, 1, 1));
	Output.push_back(ShaderParameter(ShaderParamType::RootConstant, 2, 0));
	return Output;
}

Shader_EndSimulation::Shader_EndSimulation(DeviceContext* dev) :Shader(dev)
{
	m_Shader = RHI::CreateShaderProgam();
	m_Shader->AttachAndCompileShaderFromFile("Particles\\FinishUpdate_CS", EShaderType::SHADER_COMPUTE);
}

Shader_EndSimulation::~Shader_EndSimulation()
{}

std::vector<Shader::ShaderParameter> Shader_EndSimulation::GetShaderParameters()
{
	std::vector<Shader::ShaderParameter> Output;
	Output.push_back(ShaderParameter(ShaderParamType::SRV, 0, 0));
	Output.push_back(ShaderParameter(ShaderParamType::UAV, 1, 1));
	Output.push_back(ShaderParameter(ShaderParamType::UAV, 2, 2));
	return Output;
}

Shader_ParticleEmit::Shader_ParticleEmit(DeviceContext* dev) :Shader(dev)
{
	m_Shader = RHI::CreateShaderProgam();
	m_Shader->AttachAndCompileShaderFromFile("Particles\\ParticleEmit_CS", EShaderType::SHADER_COMPUTE);
}

Shader_ParticleEmit::~Shader_ParticleEmit()
{}

std::vector<Shader::ShaderParameter> Shader_ParticleEmit::GetShaderParameters()
{
	std::vector<Shader::ShaderParameter> Output;
	Output.push_back(ShaderParameter(ShaderParamType::UAV, 0, 0));
	Output.push_back(ShaderParameter(ShaderParamType::UAV, 1, 1));
	Output.push_back(ShaderParameter(ShaderParamType::UAV, 2, 2));
	Output.push_back(ShaderParameter(ShaderParamType::UAV, 3, 3));
	return Output;
}