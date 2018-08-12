#pragma once
#include "RHI/ShaderProgramBase.h"
#include <d3d12.h>
#include "glm\glm.hpp"
#include "RHI/Shader.h"
#include "D3D12Helpers.h"
class D3D12Shader : public ShaderProgramBase
{
public:
	D3D12Shader(DeviceContext* Device);
	virtual ~D3D12Shader();
	struct ShaderBlobs
	{
		ID3DBlob*					vsBlob = nullptr;
		ID3DBlob*					fsBlob = nullptr;
		ID3DBlob*					csBlob = nullptr;
		ID3DBlob*					gsBlob = nullptr;
	};
	struct PiplineShader
	{
		ID3D12PipelineState* m_pipelineState = nullptr;
		ID3D12RootSignature* m_rootSignature = nullptr;
		bool IsCompute = false;
		bool operator!=(const PiplineShader &other) const
		{
			return (m_pipelineState != other.m_pipelineState) || (m_rootSignature != other.m_rootSignature);
		}
		void Release()
		{
			SafeRelease(m_pipelineState);
			SafeRelease(m_rootSignature);
		}
	};

	enum ComputeRootParameters : UINT32
	{
		ComputeRootCBV = 0,
		ComputeRootSRVTable,
		ComputeRootUAVTable,
		ComputeRootParametersCount
	};

	virtual EShaderError AttachAndCompileShaderFromFile(const char * filename, EShaderType type, const char * Entrypoint = "") override;
	virtual EShaderError AttachAndCompileShaderFromFile(const char * filename, EShaderType type) override;
	virtual void BuildShaderProgram() override;
	virtual void DeleteShaderProgram() override;
	virtual void ActivateShaderProgram() override;
	virtual void DeactivateShaderProgram() override;

	static D3D12Shader::PiplineShader CreateComputePipelineShader(PiplineShader & output, D3D12_INPUT_ELEMENT_DESC * inputDisc, int DescCount, ShaderBlobs * blobs, PipeLineState Depthtest, DeviceContext * context);
	static PiplineShader CreatePipelineShader(PiplineShader & output, D3D12_INPUT_ELEMENT_DESC * inputDisc, int DescCount, ShaderBlobs* blobs, PipeLineState Depthtest, DeviceContext* context);

	ShaderBlobs* GetShaderBlobs();
	static bool ParseVertexFormat(std::vector<Shader::VertexElementDESC>, D3D12_INPUT_ELEMENT_DESC** Data, int* length);
	static void CreateRootSig(D3D12Shader::PiplineShader &output, std::vector<Shader::ShaderParameter> Parms, DeviceContext* context);
	static void CreateDefaultRootSig(D3D12Shader::PiplineShader & output);
	void Init();
	void CreateComputePipelineShader();
	static D3D12_INPUT_ELEMENT_DESC ConvertVertexFormat(Shader::VertexElementDESC * desc);
	virtual void CreateShaderProgram() override;
	PiplineShader* GetPipelineShader();
private:
	D3D_SHADER_MACRO * ParseDefines();
	class DxIncludeHandler* IncludeHandler = nullptr;
	class D3D12DeviceContext* CurrentDevice = nullptr;
	PiplineShader m_Shader;
	ID3D12DescriptorHeap* m_samplerHeap = nullptr;
	ShaderBlobs mBlolbs;
};

