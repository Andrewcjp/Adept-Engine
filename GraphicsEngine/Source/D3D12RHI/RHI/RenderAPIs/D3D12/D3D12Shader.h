#pragma once
#include "D3D12RHI.h"
#include "RHI/ShaderProgramBase.h"
#include "RHI/Shader.h"
#include "D3D12Helpers.h"
#include "Core/Utils/RefChecker.h"

class D3D12PipeLineStateObject;
struct DxcDefine;
struct ShaderSourceFile;
struct RootSignitureCreateInfo
{
	bool IsLocalSig = false;
	bool IsGlobalSig = false;
};
#if USE_DIXL
typedef IDxcBlob ShaderBlob;
#else
typedef ID3DBlob ShaderBlob;
#endif
class D3D12Shader : public ShaderProgramBase
{
public:
	D3D12Shader(DeviceContext* Device);
	virtual ~D3D12Shader();
	struct ShaderBlobs
	{
		ShaderBlob*					vsBlob = nullptr;
		ShaderBlob*					fsBlob = nullptr;
		ShaderBlob*					csBlob = nullptr;
		ShaderBlob*					gsBlob = nullptr;
		ShaderBlob*					RTLibBlob = nullptr;
		ShaderBlob*					GetBlob(EShaderType::Type t);
	};

	virtual EShaderError::Type AttachAndCompileShaderFromFile(const char * filename, EShaderType::Type type, const char * Entrypoint = "") override;
	virtual EShaderError::Type AttachAndCompileShaderFromFile(const char * filename, EShaderType::Type type) override;
	static D3D12_SHADER_BYTECODE GetByteCode(ShaderBlob * b);
	static void CreateComputePipelineShader(D3D12PipeLineStateObject * output, D3D12_INPUT_ELEMENT_DESC * inputDisc, int DescCount, ShaderBlobs * blobs, const RHIPipeLineStateDesc & Depthtest, DeviceContext * context);

	static	void CreatePipelineShader(D3D12PipeLineStateObject* output, D3D12_INPUT_ELEMENT_DESC * inputDisc, int DescCount, ShaderBlobs * blobs, const RHIPipeLineStateDesc & Depthtest, DeviceContext * context);

	ShaderBlobs* GetShaderBlobs();
	static bool ParseVertexFormat(std::vector<Shader::VertexElementDESC>, D3D12_INPUT_ELEMENT_DESC** Data, int* length);
	static void CreateRootSig(ID3D12RootSignature ** output, std::vector<ShaderParameter> Params, DeviceContext * context, bool compute, std::vector<RHISamplerDesc> samplers, RootSignitureCreateInfo Info = RootSignitureCreateInfo());

	void Init();
	static D3D12_INPUT_ELEMENT_DESC ConvertVertexFormat(Shader::VertexElementDESC * desc);

#if !BUILD_SHIPPING
	static void PrintShaderStats();
#endif
	glm::ivec3 ComputeThreadSize = glm::ivec3(1, 1, 1);

	glm::ivec3 GetComputeThreadSize() const override;

private:

	

	class D3D12DeviceContext* CurrentDevice = nullptr;

	ID3D12DescriptorHeap* m_samplerHeap = nullptr;
	ShaderBlobs mBlolbs;
	ShaderBlob ** GetCurrentBlob(EShaderType::Type type);

	void ReportStats(ShaderSourceFile * ShaderData);

	static const std::string GetUniqueName(std::vector<ShaderParameter>& Params);
	static D3D12_STATIC_SAMPLER_DESC * ConvertSamplers(std::vector<RHISamplerDesc>& samplers);
#if !BUILD_SHIPPING
	struct ShaderStats
	{
		int ShaderComplieCount = 0;
		int TotalShaderCount = 0;
		int ShaderLoadFromCacheCount = 0;
	};
	static ShaderStats stats;
#endif
	
};

