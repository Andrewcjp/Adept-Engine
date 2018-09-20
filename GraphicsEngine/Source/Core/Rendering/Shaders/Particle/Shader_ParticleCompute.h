#pragma once
#include "RHI/Shader.h"
class Shader_ParticleCompute : public Shader
{
public:
	Shader_ParticleCompute(class DeviceContext* dev);
	~Shader_ParticleCompute();
	std::vector<ShaderParameter> GetShaderParameters();
	bool IsComputeShader()override
	{
		return true;
	};
	NAME_SHADER(Shader_ParticleCompute);
};

class Shader_StartSimulation : public Shader
{
public:
	Shader_StartSimulation(class DeviceContext* dev);
	~Shader_StartSimulation();
	std::vector<ShaderParameter> GetShaderParameters();
	bool IsComputeShader()override
	{
		return true;
	};
	NAME_SHADER(Shader_StartSimulation);
};

class Shader_EndSimulation : public Shader
{
public:
	Shader_EndSimulation(class DeviceContext* dev);
	~Shader_EndSimulation();
	std::vector<ShaderParameter> GetShaderParameters();
	bool IsComputeShader()override
	{
		return true;
	};
	NAME_SHADER(Shader_EndSimulation);
};

class Shader_ParticleEmit : public Shader
{
public:
	Shader_ParticleEmit(class DeviceContext* dev);
	~Shader_ParticleEmit();
	std::vector<ShaderParameter> GetShaderParameters();
	bool IsComputeShader()override
	{
		return true;
	};
	NAME_SHADER(Shader_ParticleEmit);
};