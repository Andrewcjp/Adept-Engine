#pragma once
#include "RHI/RHICommandList.h"
#include "RHI/Shader.h"

class Shader_Blur : public Shader
{
public:
	DECLARE_GLOBAL_SHADER(Shader_Blur);
	
	Shader_Blur(DeviceContext* dev) :Shader(dev)
	{
	//	m_Shader->AttachAndCompileShaderFromFile("PostProcess\\BlurCS", EShaderType::SHADER_COMPUTE);
	}
	~Shader_Blur();
	bool IsComputeShader() override;
	virtual std::vector<ShaderParameter> GetShaderParameters() override
	{
		std::vector<ShaderParameter> Output;
		Output.push_back(ShaderParameter(ShaderParamType::SRV, 0, 0));
		Output.push_back(ShaderParameter(ShaderParamType::UAV, 1, 0));
		Output.push_back(ShaderParameter(ShaderParamType::CBV, 2, 0));
		return Output;
	}
	static std::vector<float> CalcGaussWeights(float sigma);
};

class Shader_BlurVert : public Shader
{
public:
	DECLARE_GLOBAL_SHADER(Shader_BlurVert);
	Shader_BlurVert(DeviceContext* dev) :Shader(dev)
	{
	//	m_Shader->AttachAndCompileShaderFromFile("PostProcess\\BlurCS", EShaderType::SHADER_COMPUTE, "VertBlurCS");
		Blurweights = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
		Blurweights->CreateConstantBuffer(sizeof(float) * 11, 1);
		std::vector<float> Weights = Shader_Blur::CalcGaussWeights(2.5f);
		Blurweights->UpdateConstantBuffer(Weights.data(), 0);
	}
	~Shader_BlurVert()
	{
		EnqueueSafeRHIRelease(Blurweights);
	}
	bool IsComputeShader() override
	{
		return true;
	}

	std::vector<ShaderParameter> GetShaderParameters()override
	{
		std::vector<ShaderParameter> Output;
		Output.push_back(ShaderParameter(ShaderParamType::SRV, 0, 0));
		Output.push_back(ShaderParameter(ShaderParamType::UAV, 1, 0));
		Output.push_back(ShaderParameter(ShaderParamType::CBV, 2, 0));
		return Output;
	}
	RHIBuffer * Blurweights = nullptr;

};




