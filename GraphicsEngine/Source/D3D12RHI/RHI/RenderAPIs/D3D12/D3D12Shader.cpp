
#include "D3D12Shader.h"
#include "D3D12RHI.h"
#include "Core/Utils/FileUtils.h"
#include "Core/Utils/StringUtil.h"
#include "Core/Asserts.h"
#include "RHI/DeviceContext.h"
#include "D3D12DeviceContext.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Assets/AssetManager.h"
#include <d3dcompiler.h>
#include "Core/Platform/ConsoleVariable.h"
static ConsoleVariable NoShaderCache("NoShaderCache", 0, ECVarType::LaunchOnly);
#if !BUILD_SHIPPING
D3D12Shader::ShaderStats D3D12Shader::stats = D3D12Shader::ShaderStats();
#endif
D3D12Shader::D3D12Shader(DeviceContext* Device)
{
	CurrentDevice = (D3D12DeviceContext*)Device;
	CacheBlobs = !NoShaderCache.GetBoolValue();
	if (D3D12RHI::DetectGPUDebugger())
	{
		CacheBlobs = false;
	}
	if (!CacheBlobs)
	{
		Log::LogMessage("Shader Cache Disabled",Log::Warning);
	}
}

D3D12Shader::~D3D12Shader()
{
	CurrentDevice = nullptr;
	SafeRelease(m_Shader.m_rootSignature);
	SafeRelease(m_Shader.m_pipelineState);
	SafeRelease(mBlolbs.csBlob);
	SafeRelease(mBlolbs.fsBlob);
	SafeRelease(mBlolbs.vsBlob);
	SafeRelease(mBlolbs.gsBlob);
}

void StripD3DShader(ID3DBlob** blob)
{
#if !BUILD_SHIPPING
	return;
#else
	if (*blob == nullptr)
	{
		return;
	}
	UINT stripflags = D3DCOMPILER_STRIP_REFLECTION_DATA | D3DCOMPILER_STRIP_DEBUG_INFO | D3DCOMPILER_STRIP_TEST_BLOBS;
	D3DStripShader(*blob, (*blob)->GetBufferSize(), stripflags, blob);
#endif
}

D3D_SHADER_MACRO* D3D12Shader::ParseDefines()
{
	D3D_SHADER_MACRO* out = new D3D_SHADER_MACRO[Defines.size() + 1];
	for (int i = 0; i < Defines.size(); i++)//array is set up as Name, Value
	{
		out[i].Name = Defines[i].Name.c_str();
		out[i].Definition = Defines[i].Value.c_str();
	}
	int last = (int)Defines.size();
	out[last].Definition = NULL;
	out[last].Name = NULL;
	return out;
}

EShaderError::Type D3D12Shader::AttachAndCompileShaderFromFile(const char * shadername, EShaderType::Type type)
{
	return AttachAndCompileShaderFromFile(shadername, type, "main");
}

ID3DBlob** D3D12Shader::GetCurrentBlob(EShaderType::Type type)
{
	switch (type)
	{
	case EShaderType::SHADER_VERTEX:
		return &mBlolbs.vsBlob;
		break;
	case EShaderType::SHADER_FRAGMENT:
		return &mBlolbs.fsBlob;
		break;
	case EShaderType::SHADER_GEOMETRY:
		return &mBlolbs.gsBlob;
		break;
	case EShaderType::SHADER_COMPUTE:
		return &mBlolbs.csBlob;
		break;
	case EShaderType::SHADER_UNDEFINED:
		break;
	default:
		break;
	}
	return nullptr;
}

const std::string D3D12Shader::GetShaderInstanceHash()
{
	if (Defines.size() == 0)
	{
		return "";
	}
	std::string DefineSum;
	for (Shader_Define d : Defines)
	{
		DefineSum += d.Name + d.Value;
	}
	size_t Hash = std::hash<std::string>{} (DefineSum);
	return "_" + std::to_string(Hash);
}

EShaderError::Type D3D12Shader::AttachAndCompileShaderFromFile(const char * shadername, EShaderType::Type ShaderType, const char * Entrypoint)
{
	SCOPE_STARTUP_COUNTER("Shader Compile");
#if !BUILD_SHIPPING
	stats.TotalShaderCount++;
#endif
	if (TryLoadCachedShader(shadername, GetCurrentBlob(ShaderType), GetShaderInstanceHash(), ShaderType))
	{
#if !BUILD_SHIPPING
		stats.ShaderLoadFromCacheCount++;
#endif
		return EShaderError::SHADER_ERROR_NONE;
	}

	std::string path = AssetManager::GetShaderPath();
	std::string name = shadername;
	name.append(".hlsl");
	path.append(name);

	if (!FileUtils::File_ExistsTest(path))
	{
		__debugbreak();
		return EShaderError::SHADER_ERROR_NOFILE;
	}
	if (ShaderType == EShaderType::SHADER_COMPUTE)
	{
		IsCompute = true;
	}

	std::string ShaderData = AssetManager::Get()->LoadFileWithInclude(name);
	LPCWSTR filename = StringUtils::ConvertStringToWide(path).c_str();
	ID3DBlob* pErrorBlob = NULL;
	HRESULT hr = S_OK;
	UINT compileFlags = 0;
	if (ShaderComplier::Get()->ShouldBuildDebugShaders())
	{
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_ALL_RESOURCES_BOUND;//D3DCOMPILE_PARTIAL_PRECISION
	}
	else
	{
		UINT compileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_ALL_RESOURCES_BOUND | D3DCOMPILE_ENABLE_STRICTNESS;
		//	compileFlags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
	}
	D3D_SHADER_MACRO* defines = ParseDefines();
	switch (ShaderType)
	{
	case EShaderType::SHADER_VERTEX:
		hr = D3DCompile(ShaderData.c_str(), ShaderData.size(), shadername, defines, nullptr, Entrypoint, "vs_5_0",
			compileFlags, 0, &mBlolbs.vsBlob, &pErrorBlob);
		break;
	case EShaderType::SHADER_FRAGMENT:
		hr = D3DCompile(ShaderData.c_str(), ShaderData.size(), shadername, defines, nullptr, Entrypoint, "ps_5_1",
			compileFlags, 0, &mBlolbs.fsBlob, &pErrorBlob);
		break;
	case EShaderType::SHADER_GEOMETRY:
		hr = D3DCompile(ShaderData.c_str(), ShaderData.size(), shadername, defines, nullptr, Entrypoint, "gs_5_0",
			compileFlags, 0, &mBlolbs.gsBlob, &pErrorBlob);
		break;
	case EShaderType::SHADER_COMPUTE:
		hr = D3DCompile(ShaderData.c_str(), ShaderData.size(), shadername, defines, nullptr, Entrypoint, "cs_5_0",
			compileFlags, 0, &mBlolbs.csBlob, &pErrorBlob);
		break;
	case EShaderType::SHADER_UNDEFINED:
	default:
		ensureMsgf(false, "Unknown Shader Type!");
		break;
	}

	StripD3DShader(GetCurrentBlob(ShaderType));
	if (pErrorBlob)
	{
		std::string Log = "Shader Compile Output: ";
		Log.append(name);
		Log.append("\n");
		Log.append(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));

		if (FAILED(hr))
		{
			Log::LogMessage(Log, Log::Severity::Error);
			PlatformApplication::DisplayMessageBox("Shader Complie Error", Log);
			pErrorBlob->Release();
#ifndef NDEBUG
			__debugbreak();
#endif
			Engine::RequestExit(-1);
			return EShaderError::SHADER_ERROR_COMPILE;
		}
		else
		{
			Log::LogMessage(Log, Log::Severity::Warning);
		}
	}

	if (pErrorBlob)
	{
		pErrorBlob->Release();
	}
	if (FAILED(hr))
	{
		return EShaderError::SHADER_ERROR_CREATE;
	}
	WriteBlobs(shadername, ShaderType);
#if !BUILD_SHIPPING
	stats.ShaderComplieCount++;
#endif
	return EShaderError::SHADER_ERROR_NONE;
}

bool D3D12Shader::CompareCachedShaderBlobWithSRC(const std::string & ShaderName, const std::string & FullShaderName)
{
	std::string ShaderSRCPath = AssetManager::GetShaderPath() + ShaderName + ".hlsl";
	std::string ShaderCSOPath = AssetManager::GetDDCPath() + "Shaders\\" + FullShaderName;
	//if the source is newer than the CSO recompile
	return PlatformApplication::CheckFileSrcNewer(ShaderSRCPath, ShaderCSOPath);
}

const std::string D3D12Shader::GetShaderNamestr(const std::string & Shadername, const std::string & InstanceHash, EShaderType::Type type)
{
	std::string OutputName = Shadername;
	OutputName += "_" + std::to_string((int)type);
	OutputName += "_" + InstanceHash;
	if (ShaderComplier::Get()->ShouldBuildDebugShaders())
	{
		OutputName += "_D";
	}
	OutputName += ".cso";
	return OutputName;
}

bool D3D12Shader::TryLoadCachedShader(std::string Name, ID3DBlob ** Blob, const std::string & InstanceHash, EShaderType::Type type)
{
	if (!CacheBlobs)
	{
		return false;
	}
	const std::string FullShaderName = GetShaderNamestr(Name, InstanceHash, type);
	std::string ShaderPath = AssetManager::GetDDCPath() + "Shaders\\" + FullShaderName;
#if BUILD_PACKAGE
	ensureFatalMsgf(FileUtils::File_ExistsTest(ShaderPath), "Missing shader: " + GetShaderNamestr(Name, InstanceHash, type));
	ThrowIfFailed(D3DReadFileToBlob(StringUtils::ConvertStringToWide(ShaderPath).c_str(), Blob));
	return true;
#else	
	if (FileUtils::File_ExistsTest(ShaderPath) && CompareCachedShaderBlobWithSRC(Name, FullShaderName))
	{
		ThrowIfFailed(D3DReadFileToBlob(StringUtils::ConvertStringToWide(ShaderPath).c_str(), Blob));
		return true;
	}
	return false;
#endif
}

void D3D12Shader::WriteBlobs(const std::string & shadername, EShaderType::Type type)
{
	if (CacheBlobs)
	{
		const std::string DDcShaderPath = AssetManager::GetDDCPath() + "Shaders\\";
		FileUtils::CreateDirectoriesToFullPath(DDcShaderPath + shadername + ".");
		ThrowIfFailed(D3DWriteBlobToFile(*GetCurrentBlob(type), StringUtils::ConvertStringToWide(DDcShaderPath + GetShaderNamestr(shadername, GetShaderInstanceHash(), type)).c_str(), true));
	}
}

D3D12PiplineShader D3D12Shader::CreateComputePipelineShader(D3D12PiplineShader &output, D3D12_INPUT_ELEMENT_DESC* inputDisc, int DescCount, ShaderBlobs* blobs, PipeLineState Depthtest,
	DeviceContext* context)
{
	SCOPE_STARTUP_COUNTER("Create Compute PSO");
	D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
	computePsoDesc.pRootSignature = output.m_rootSignature;
	computePsoDesc.CS = CD3DX12_SHADER_BYTECODE(blobs->csBlob);
	ThrowIfFailed(((D3D12DeviceContext*)context)->GetDevice()->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&output.m_pipelineState)));
	return output;
}

D3D12PiplineShader D3D12Shader::CreatePipelineShader(D3D12PiplineShader &output, D3D12_INPUT_ELEMENT_DESC* inputDisc, int DescCount, ShaderBlobs* blobs, PipeLineState Depthtest,
	DeviceContext* context)
{
	SCOPE_STARTUP_COUNTER("Create PSO");
	ensure(blobs->vsBlob != nullptr);
	ensure(blobs->fsBlob != nullptr);
	if (context == nullptr)
	{
		context = RHI::GetDeviceContext(0);
	}
	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = { 0 };
	psoDesc.InputLayout.pInputElementDescs = inputDisc;
	psoDesc.InputLayout.NumElements = DescCount;
	psoDesc.pRootSignature = output.m_rootSignature;
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(blobs->vsBlob);
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(blobs->fsBlob);
	if (blobs->gsBlob != nullptr)
	{
		psoDesc.GS = CD3DX12_SHADER_BYTECODE(blobs->gsBlob);
	}
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	if (Depthtest.RasterMode == PRIMITIVE_TOPOLOGY_TYPE::PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
	{
		//psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON;
	}
	psoDesc.RasterizerState.CullMode = Depthtest.Cull ? D3D12_CULL_MODE_BACK : D3D12_CULL_MODE_NONE;
	if (Depthtest.Blending)
	{
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.BlendState.AlphaToCoverageEnable = true;
		psoDesc.BlendState.IndependentBlendEnable = FALSE;
		psoDesc.BlendState.RenderTarget[0].BlendEnable = TRUE;
	}
	else
	{
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	}
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = Depthtest.DepthTest;
	psoDesc.DepthStencilState.DepthFunc = (D3D12_COMPARISON_FUNC)Depthtest.DepthCompareFunction;
	psoDesc.DepthStencilState.DepthWriteMask = Depthtest.DepthWrite ? D3D12_DEPTH_WRITE_MASK::D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK::D3D12_DEPTH_WRITE_MASK_ZERO;
	psoDesc.DepthStencilState.StencilEnable = false;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = (D3D12_PRIMITIVE_TOPOLOGY_TYPE)Depthtest.RasterMode;
	psoDesc.NumRenderTargets = Depthtest.RenderTargetDesc.NumRenderTargets;
	psoDesc.SampleDesc.Count = 1;

	for (int i = 0; i < 8; i++)
	{
		psoDesc.RTVFormats[i] = D3D12Helpers::ConvertFormat(Depthtest.RenderTargetDesc.RTVFormats[i]);
	}
	psoDesc.DSVFormat = D3D12Helpers::ConvertFormat(Depthtest.RenderTargetDesc.DSVFormat);

	ThrowIfFailed(((D3D12DeviceContext*)context)->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&output.m_pipelineState)));
	return output;
}

D3D12Shader::ShaderBlobs * D3D12Shader::GetShaderBlobs()
{
	return &mBlolbs;
}

void D3D12Shader::Init()
{
	if (IsCompute)
	{
		CreateComputePipelineShader();
	}
}

void D3D12Shader::CreateComputePipelineShader()
{
	//The compute shader expects 2 floats, the source texture and the destination texture
	CD3DX12_DESCRIPTOR_RANGE srvCbvRanges[2];
	CD3DX12_ROOT_PARAMETER rootParameters[3];
	srvCbvRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
	srvCbvRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0);
	rootParameters[0].InitAsConstants(2, 0);
	rootParameters[1].InitAsDescriptorTable(1, &srvCbvRanges[0]);
	rootParameters[2].InitAsDescriptorTable(1, &srvCbvRanges[1]);

	//Static sampler used to get the linearly interpolated color for the mipmaps
	D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
	samplerDesc.ShaderRegister = 0;
	samplerDesc.RegisterSpace = 0;
	samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	CD3DX12_ROOT_SIGNATURE_DESC computeRootSignatureDesc;
	//computeRootSignatureDesc.Init(_countof(rootParameters), rootParameters, 0, nullptr);

	ID3DBlob* signature;
	ID3DBlob* error;
	computeRootSignatureDesc.Init(_countof(rootParameters), rootParameters, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	ThrowIfFailed(D3D12SerializeRootSignature(&computeRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
	ThrowIfFailed(CurrentDevice->GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_Shader.m_rootSignature)));
	NAME_D3D12_OBJECT(m_Shader.m_rootSignature);

	D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
	computePsoDesc.pRootSignature = m_Shader.m_rootSignature;
	computePsoDesc.CS = CD3DX12_SHADER_BYTECODE(mBlolbs.csBlob);
	ThrowIfFailed(CurrentDevice->GetDevice()->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&m_Shader.m_pipelineState)));
}

D3D12_INPUT_ELEMENT_DESC D3D12Shader::ConvertVertexFormat(Shader::VertexElementDESC* desc)
{
	D3D12_INPUT_ELEMENT_DESC output;
	output.SemanticName = desc->SemanticName;
	output.AlignedByteOffset = desc->AlignedByteOffset;
	output.InputSlot = desc->InputSlot;
	output.SemanticIndex = desc->SemanticIndex;
	output.Format = D3D12Helpers::ConvertFormat(desc->Format);
	output.InstanceDataStepRate = desc->InstanceDataStepRate;
	//identical!
	output.InputSlotClass = (D3D12_INPUT_CLASSIFICATION)desc->InputSlotClass;
	return output;
}

D3D12PiplineShader * D3D12Shader::GetPipelineShader()
{
	return &m_Shader;
}
#if !BUILD_SHIPPING
void D3D12Shader::PrintShaderStats()
{
	std::stringstream ss;
	ss << "Shader Stats: Loaded: " << stats.ShaderLoadFromCacheCount << "/" << stats.TotalShaderCount << " Complied: " << stats.ShaderComplieCount << "/" << stats.TotalShaderCount;
	Log::LogMessage(ss.str());
}
#endif
bool D3D12Shader::ParseVertexFormat(std::vector<Shader::VertexElementDESC> desc, D3D12_INPUT_ELEMENT_DESC ** Data, int * length)
{
	*Data = new D3D12_INPUT_ELEMENT_DESC[desc.size()];
	*length = (int)desc.size();
	D3D12_INPUT_ELEMENT_DESC* Dataptr = *Data;
	for (int i = 0; i < desc.size(); i++)
	{
		Dataptr[i] = ConvertVertexFormat(&desc[i]);
	}
	return true;
}

void D3D12Shader::CreateRootSig(D3D12PiplineShader &output, std::vector<Shader::ShaderParameter> Params, DeviceContext* context)
{
	SCOPE_STARTUP_COUNTER("CreateRootSig");
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

	// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if (FAILED(((D3D12DeviceContext*)context)->GetDevice()->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}
#define UAVRANGES 1
	//cbvs will be InitAsConstantBufferView
	//srvs and UAvs will be in Ranges
	CD3DX12_ROOT_PARAMETER1* rootParameters = nullptr;
	rootParameters = new CD3DX12_ROOT_PARAMETER1[Params.size()];
	int RangeNumber = 0;
	for (int i = 0; i < Params.size(); i++)
	{
		rootParameters[i] = CD3DX12_ROOT_PARAMETER1();
		if (Params[i].Type == Shader::ShaderParamType::SRV)
		{
			RangeNumber++;
		}
#if UAVRANGES
		else if (Params[i].Type == Shader::ShaderParamType::UAV)
		{
			RangeNumber++;
		}
#endif
	}
	D3D12_SHADER_VISIBILITY BaseSRVVis = D3D12_SHADER_VISIBILITY_ALL;
	if (output.IsCompute)
	{
		BaseSRVVis = D3D12_SHADER_VISIBILITY::D3D12_SHADER_VISIBILITY_ALL;
	}
	CD3DX12_DESCRIPTOR_RANGE1* ranges = nullptr;
	if (RangeNumber > 0)
	{
		ranges = new CD3DX12_DESCRIPTOR_RANGE1[Params.size()];
	}
	for (int i = 0; i < Params.size(); i++)
	{
		if (Params[i].Type == Shader::ShaderParamType::SRV)
		{
			ranges[Params[i].SignitureSlot].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, Params[i].NumDescriptors, Params[i].RegisterSlot, Params[i].RegisterSpace, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, 0);
			rootParameters[Params[i].SignitureSlot].InitAsDescriptorTable(1, &ranges[Params[i].SignitureSlot], output.IsCompute ? BaseSRVVis : (D3D12_SHADER_VISIBILITY)Params[i].Visiblity);
		}
		else if (Params[i].Type == Shader::ShaderParamType::CBV)
		{
			rootParameters[Params[i].SignitureSlot].InitAsConstantBufferView(Params[i].RegisterSlot, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
		}
		else if (Params[i].Type == Shader::ShaderParamType::UAV)
		{
#if !UAVRANGES
			rootParameters[Params[i].SignitureSlot].InitAsUnorderedAccessView(Params[i].RegisterSlot, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
#else
			ranges[Params[i].SignitureSlot].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, Params[i].NumDescriptors, Params[i].RegisterSlot, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, 0);
			rootParameters[Params[i].SignitureSlot].InitAsDescriptorTable(1, &ranges[Params[i].SignitureSlot], D3D12_SHADER_VISIBILITY_ALL);
#endif
		}
		else if (Params[i].Type == Shader::ShaderParamType::RootConstant)
		{
			rootParameters[Params[i].SignitureSlot].InitAsConstants(Params[i].NumDescriptors, Params[i].RegisterSlot, Params[i].RegisterSpace, (D3D12_SHADER_VISIBILITY)Params[i].Visiblity);
		}
	}
	//todo: Samplers

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
#define NUMSamples 3
	D3D12_STATIC_SAMPLER_DESC samplers[NUMSamples];

	D3D12_STATIC_SAMPLER_DESC sampler = {};
	//	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.MipLODBias = 0;
	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	samplers[0] = sampler;

	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.ShaderRegister = 1;
	sampler.RegisterSpace = 0;
	samplers[1] = sampler;

	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
	sampler.ShaderRegister = 2;
	sampler.RegisterSpace = 0;
	samplers[2] = sampler;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc((UINT)Params.size(), rootParameters, NUMSamples, &samplers[0], D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ID3DBlob* signature = nullptr;
	ID3DBlob* pErrorBlob = nullptr;
	HRESULT hr =  (D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &pErrorBlob));
	std::string Log = "Serialize Root Signature Compile Output: ";
	if (pErrorBlob != nullptr)
	{
		Log.append(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
		if (FAILED(hr))
		{
			Log::LogMessage(Log, Log::Severity::Error);
			PlatformApplication::DisplayMessageBox("Shader Complie Error", Log);
			pErrorBlob->Release();
#ifndef NDEBUG
			__debugbreak();
#endif
			Engine::RequestExit(-1);
		}
		else
		{
			Log::LogMessage(Log, Log::Severity::Warning);
		}
	}
	ThrowIfFailed(((D3D12DeviceContext*)context)->GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&output.m_rootSignature)));

	output.m_rootSignature->SetName(StringUtils::ConvertStringToWide(GetUniqueName(Params)).c_str());
}
const std::string D3D12Shader::GetUniqueName(std::vector<Shader::ShaderParameter>& Params)
{
	std::string output = "Root sig Length = ";;
	output.append(std::to_string(Params.size()));
	for (Shader::ShaderParameter sp : Params)
	{
		output += "T";
		switch (sp.Type)
		{
		case Shader::ShaderParamType::CBV:
			output += "CBV";
			break;
		case Shader::ShaderParamType::SRV:
			output += "SRV";
			break;
		case Shader::ShaderParamType::UAV:
			output += "UAV";
			break;
		default:
			break;
		}
		output += ", ";
	}
	return output;
}

void D3D12Shader::CreateDefaultRootSig(D3D12PiplineShader &output)
{
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

	// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	/*if (FAILED(D3D12RHI::GetDevice()->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
	{
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
	}*/

	CD3DX12_DESCRIPTOR_RANGE1 ranges[2];
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
	ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);

	CD3DX12_ROOT_PARAMETER1 rootParameters[5];
	rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);
	//	rootParameters[1].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[2].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[3].InitAsConstantBufferView(2, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[4].InitAsDescriptorTable(1, &ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);
	//rootParameters[3].InitAsShaderResourceView(4, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC, D3D12_SHADER_VISIBILITY_ALL);
	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
	D3D12_STATIC_SAMPLER_DESC samplers[2];

	D3D12_STATIC_SAMPLER_DESC sampler = {};
	//	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.MipLODBias = 0;

	sampler.MaxAnisotropy = 0;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderRegister = 0;
	sampler.RegisterSpace = 0;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	samplers[0] = sampler;


	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.ShaderRegister = 1;
	sampler.RegisterSpace = 0;
	samplers[1] = sampler;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc(_countof(rootParameters), rootParameters, 2, &samplers[0], D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ID3DBlob* signature;
	ID3DBlob* error;
	ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &error));
	//ThrowIfFailed(D3D12RHI::GetDisplayDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&output.m_rootSignature)));
}

D3D12PiplineShader::D3D12PiplineShader()
{
	//AddCheckerRef(D3D12PiplineShader, this);
}

void D3D12PiplineShader::Release()
{
	SafeRelease(m_pipelineState);
	SafeRelease(m_rootSignature);
}
