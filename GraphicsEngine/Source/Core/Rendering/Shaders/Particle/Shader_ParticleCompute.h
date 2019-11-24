#pragma once
#include "RHI/Shader.h"
class Shader_ParticleCompute : public Shader
{
public:
	DECLARE_GLOBAL_SHADER(Shader_ParticleCompute);
	Shader_ParticleCompute(class DeviceContext* dev);
	~Shader_ParticleCompute();
	bool IsComputeShader()override
	{
		return true;
	};

};

class Shader_StartSimulation : public Shader
{
public:
	DECLARE_GLOBAL_SHADER(Shader_StartSimulation);
	Shader_StartSimulation(class DeviceContext* dev);
	~Shader_StartSimulation();
	std::vector<ShaderParameter> GetShaderParameters();
	bool IsComputeShader()override
	{
		return true;
	};
};

class Shader_EndSimulation : public Shader
{
public:
	DECLARE_GLOBAL_SHADER(Shader_EndSimulation);
	Shader_EndSimulation(class DeviceContext* dev);
	~Shader_EndSimulation();
	std::vector<ShaderParameter> GetShaderParameters();
	bool IsComputeShader()override
	{
		return true;
	};
};

class Shader_ParticleEmit : public Shader
{
public:
	DECLARE_GLOBAL_SHADER(Shader_ParticleEmit);
	Shader_ParticleEmit(class DeviceContext* dev);
	~Shader_ParticleEmit();
	std::vector<ShaderParameter> GetShaderParameters();
	bool IsComputeShader()override
	{
		return true;
	};

};