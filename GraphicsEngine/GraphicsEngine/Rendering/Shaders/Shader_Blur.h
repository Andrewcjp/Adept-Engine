#pragma once
#include "RHI/Shader.h"
#include <vector>
class Shader_Blur : public Shader
{
public:
	Shader_Blur()
	{
		m_Shader = RHI::CreateShaderProgam(RHI::GetDeviceContext(0));
		m_Shader->CreateShaderProgram();
		m_Shader->AttachAndCompileShaderFromFile("BlurCS", EShaderType::SHADER_COMPUTE);
	}
	~Shader_Blur();
	bool IsComputeShader() override;
	virtual std::vector<ShaderParameter> GetShaderParameters() override
	{
		std::vector<Shader::ShaderParameter> Output;
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
	Shader_BlurVert()
	{
		m_Shader = RHI::CreateShaderProgam(RHI::GetDeviceContext(0));
		m_Shader->CreateShaderProgram();
		m_Shader->AttachAndCompileShaderFromFile("BlurCS", EShaderType::SHADER_COMPUTE,"VertBlurCS");
		Blurweights = RHI::CreateRHIBuffer(RHIBuffer::BufferType::Constant);
		Blurweights->CreateConstantBuffer(sizeof(float) * 11, 1);
		std::vector<float> Weights = Shader_Blur::CalcGaussWeights(2.5f);
		int blurRadius = (int)Weights.size() / 2;
		//cmdList->SetComputeRoot32BitConstants(0, 1, &blurRadius, 0);
		Blurweights->UpdateConstantBuffer(Weights.data(), 0);
	}

	bool IsComputeShader() override
	{
		return true;
	}
	std::vector<Shader::ShaderParameter> GetShaderParameters()override
	{
		std::vector<Shader::ShaderParameter> Output;
		Output.push_back(ShaderParameter(ShaderParamType::SRV, 0, 0));
		Output.push_back(ShaderParameter(ShaderParamType::UAV, 1, 0));
		Output.push_back(ShaderParameter(ShaderParamType::CBV, 2, 0));
		return Output;
	}
	RHIBuffer * Blurweights = nullptr;

};




