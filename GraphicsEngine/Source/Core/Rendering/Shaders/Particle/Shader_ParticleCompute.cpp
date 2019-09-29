
#include "Shader_ParticleCompute.h"
IMPLEMENT_GLOBAL_SHADER(Shader_ParticleCompute);
IMPLEMENT_GLOBAL_SHADER(Shader_StartSimulation);
IMPLEMENT_GLOBAL_SHADER(Shader_EndSimulation);
IMPLEMENT_GLOBAL_SHADER(Shader_ParticleEmit);
Shader_ParticleCompute::Shader_ParticleCompute(DeviceContext* dev) :Shader(dev)
{
	m_Shader->AttachAndCompileShaderFromFile("Particles\\ParticleSimulate_CS", EShaderType::SHADER_COMPUTE);
}

Shader_ParticleCompute::~Shader_ParticleCompute()
{}

std::vector<ShaderParameter> Shader_ParticleCompute::GetShaderParameters()
{
	ChangeParamType("emitData", ShaderParamType::RootConstant);

	return Shader::GetShaderParameters();
}

Shader_StartSimulation::Shader_StartSimulation(DeviceContext* dev) :Shader(dev)
{
	m_Shader->AttachAndCompileShaderFromFile("Particles\\StartUpdate_CS", EShaderType::SHADER_COMPUTE);
}

Shader_StartSimulation::~Shader_StartSimulation()
{}

std::vector<ShaderParameter> Shader_StartSimulation::GetShaderParameters()
{
	std::vector<ShaderParameter> Output;
	Output.push_back(ShaderParameter(ShaderParamType::UAV, 0, 0)); 
	Output.push_back(ShaderParameter(ShaderParamType::UAV, 1, 1));
	Output.push_back(ShaderParameter(ShaderParamType::RootConstant, 2, 0));
	return Output;
}

Shader_EndSimulation::Shader_EndSimulation(DeviceContext* dev) :Shader(dev)
{
	m_Shader->AttachAndCompileShaderFromFile("Particles\\FinishUpdate_CS", EShaderType::SHADER_COMPUTE);
}

Shader_EndSimulation::~Shader_EndSimulation()
{}

std::vector<ShaderParameter> Shader_EndSimulation::GetShaderParameters()
{
	return Shader::GetShaderParameters();
}

Shader_ParticleEmit::Shader_ParticleEmit(DeviceContext* dev) :Shader(dev)
{
	m_Shader->AttachAndCompileShaderFromFile("Particles\\ParticleEmit_CS", EShaderType::SHADER_COMPUTE);
}

Shader_ParticleEmit::~Shader_ParticleEmit()
{}

std::vector<ShaderParameter> Shader_ParticleEmit::GetShaderParameters()
{
	std::vector<ShaderParameter> Output;
	Output.push_back(ShaderParameter(ShaderParamType::UAV, 0, 0));
	Output.push_back(ShaderParameter(ShaderParamType::UAV, 1, 1));
	Output.push_back(ShaderParameter(ShaderParamType::UAV, 2, 2));
	Output.push_back(ShaderParameter(ShaderParamType::UAV, 3, 3));
	return Output;
}