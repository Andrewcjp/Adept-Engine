#pragma once
#include "RHI/ShaderProgramBase.h"
#include <d3d12.h>
#include "glm\glm.hpp"
#include "RHI/Shader.h"
#include "D3D12Helpers.h"
#include "Core/Utils/RefChecker.h"

struct D3D12PiplineShader
{
	D3D12PiplineShader();
	ID3D12PipelineState* m_pipelineState = nullptr;
	ID3D12RootSignature* m_rootSignature = nullptr;
	bool IsCompute = false;
	bool operator!=(const D3D12PiplineShader &other) const
	{
		return (m_pipelineState != other.m_pipelineState) || (m_rootSignature != other.m_rootSignature);
	}
	void Release();
};

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

	virtual EShaderError::Type AttachAndCompileShaderFromFile(const char * filename, EShaderType::Type type, const char * Entrypoint = "") override;
	virtual EShaderError::Type AttachAndCompileShaderFromFile(const char * filename, EShaderType::Type type) override;
	static D3D12PiplineShader CreateComputePipelineShader(D3D12PiplineShader & output, D3D12_INPUT_ELEMENT_DESC * inputDisc, int DescCount, ShaderBlobs * blobs, PipeLineState Depthtest, DeviceContext * context);
	static D3D12PiplineShader CreatePipelineShader(D3D12PiplineShader & output, D3D12_INPUT_ELEMENT_DESC * inputDisc, int DescCount, ShaderBlobs* blobs, PipeLineState Depthtest, DeviceContext* context);

	ShaderBlobs* GetShaderBlobs();
	static bool ParseVertexFormat(std::vector<Shader::VertexElementDESC>, D3D12_INPUT_ELEMENT_DESC** Data, int* length);
	static void CreateRootSig(D3D12PiplineShader &output, std::vector<Shader::ShaderParameter> Parms, DeviceContext* context);
	
	static void CreateDefaultRootSig(D3D12PiplineShader & output);
	void Init();
	void CreateComputePipelineShader();
	static D3D12_INPUT_ELEMENT_DESC ConvertVertexFormat(Shader::VertexElementDESC * desc);
	D3D12PiplineShader* GetPipelineShader();
private:
	D3D_SHADER_MACRO * ParseDefines();
	class D3D12DeviceContext* CurrentDevice = nullptr;
	D3D12PiplineShader m_Shader;
	ID3D12DescriptorHeap* m_samplerHeap = nullptr;
	ShaderBlobs mBlolbs;
	bool CacheBlobs = true;
	void WriteBlobs(const std::string & shadername, EShaderType::Type type);
	bool TryLoadCachedShader(std::string Name, ID3DBlob** Blob, const std::string & InstanceHash);
	bool CompareCachedShaderBlobWithSRC(const std::string & ShaderName, const std::string & InstanceHash);
	const std::string GetShaderNamestr(const std::string & Shadername, const std::string & InstanceHash);
	ID3DBlob ** GetCurrentBlob(EShaderType::Type type);
	const std::string GetShaderInstanceHash();
	static const std::string GetUniqueName(std::vector<Shader::ShaderParameter>& Params);
};

