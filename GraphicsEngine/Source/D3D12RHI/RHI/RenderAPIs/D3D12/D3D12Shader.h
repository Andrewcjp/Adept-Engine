#pragma once
#include "D3D12RHI.h"
#include "RHI/ShaderProgramBase.h"
#include "RHI/Shader.h"
#include "D3D12Helpers.h"
#include "Core/Utils/RefChecker.h"
#include <dxcapi.h>

class D3D12PipeLineStateObject;
struct DxcDefine;
class D3D12Shader : public ShaderProgramBase
{
public:
	D3D12Shader(DeviceContext* Device);
	virtual ~D3D12Shader();
	struct ShaderBlobs
	{
		IDxcBlob*					vsBlob = nullptr;
		IDxcBlob*					fsBlob = nullptr;
		IDxcBlob*					csBlob = nullptr;
		IDxcBlob*					gsBlob = nullptr;
		IDxcBlob*					RTLibBlob = nullptr;
		IDxcBlob*					GetBlob(EShaderType::Type t);
	};

	virtual EShaderError::Type AttachAndCompileShaderFromFile(const char * filename, EShaderType::Type type, const char * Entrypoint = "") override;
	virtual EShaderError::Type AttachAndCompileShaderFromFile(const char * filename, EShaderType::Type type) override;
	static D3D12_SHADER_BYTECODE GetByteCode(IDxcBlob * b);
	static void CreateComputePipelineShader(D3D12PipeLineStateObject * output, D3D12_INPUT_ELEMENT_DESC * inputDisc, int DescCount, ShaderBlobs * blobs, const RHIPipeLineStateDesc & Depthtest, DeviceContext * context);

	static	void CreatePipelineShader(D3D12PipeLineStateObject* output, D3D12_INPUT_ELEMENT_DESC * inputDisc, int DescCount, ShaderBlobs * blobs, const RHIPipeLineStateDesc & Depthtest, DeviceContext * context);

	ShaderBlobs* GetShaderBlobs();
	static bool ParseVertexFormat(std::vector<Shader::VertexElementDESC>, D3D12_INPUT_ELEMENT_DESC** Data, int* length);
	static void CreateRootSig(D3D12PipeLineStateObject * output, std::vector<ShaderParameter> Params, DeviceContext * context, bool compute, std::vector<RHISamplerDesc> samplers);

	void Init();
	static D3D12_INPUT_ELEMENT_DESC ConvertVertexFormat(Shader::VertexElementDESC * desc);

#if !BUILD_SHIPPING
	static void PrintShaderStats();
#endif
private:
	D3D_SHADER_MACRO * ParseDefinesSM5();
	DxcDefine * ParseDefinesDXC();
	class D3D12DeviceContext* CurrentDevice = nullptr;

	ID3D12DescriptorHeap* m_samplerHeap = nullptr;
	ShaderBlobs mBlolbs;
	bool CacheBlobs = true;
	void WriteBlobs(const std::string & shadername, EShaderType::Type type);
	bool TryLoadCachedShader(const std::string& Name, IDxcBlob** Blob, const std::string & InstanceHash, EShaderType::Type type);
	bool CompareCachedShaderBlobWithSRC(const std::string & ShaderName, const std::string & InstanceHash);
	const std::string GetShaderNamestr(const std::string & Shadername, const std::string & InstanceHash, EShaderType::Type type);
	IDxcBlob ** GetCurrentBlob(EShaderType::Type type);
	const std::string GetShaderInstanceHash();

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

