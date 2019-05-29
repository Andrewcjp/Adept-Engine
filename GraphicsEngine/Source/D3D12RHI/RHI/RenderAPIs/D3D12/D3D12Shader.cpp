#include "D3D12Shader.h"
#include "Core/Assets/AssetManager.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Platform/ConsoleVariable.h"
#include "Core/Utils/FileUtils.h"
#include "D3D12DeviceContext.h"
#include "D3D12RHI.h"

#include "D3D12CommandList.h"
#include "ShaderReflection.h"
#include <atlbase.h>


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
#if !WITH_EDITOR
	CacheBlobs = true;
#endif
	if (!CacheBlobs)
	{
		Log::LogMessage("Shader Cache Disabled", Log::Warning);
	}
}

D3D12Shader::~D3D12Shader()
{
	CurrentDevice = nullptr;
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

D3D_SHADER_MACRO* D3D12Shader::ParseDefinesSM5()
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

LPCWSTR GetCopyStr(std::string data)
{
	std::wstring t = StringUtils::ConvertStringToWide(data);
	wchar_t* Data = new wchar_t[t.size() + 1];
	t.copy(Data, t.size());
	Data[t.size()] = L'\0';
	return Data;
}

DxcDefine* D3D12Shader::ParseDefinesDXC()
{
	if (Defines.size() == 0)
	{
		return nullptr;
	}
	DxcDefine* out = new DxcDefine[Defines.size() + 1];
	for (int i = 0; i < Defines.size(); i++)//array is set up as Name, Value
	{
		out[i].Name = GetCopyStr(Defines[i].Name);
		out[i].Value = GetCopyStr(Defines[i].Value);
	}
	int last = (int)Defines.size();
	out[last].Value = NULL;
	out[last].Name = NULL;
	return out;
}

EShaderError::Type D3D12Shader::AttachAndCompileShaderFromFile(const char * shadername, EShaderType::Type type)
{
	return AttachAndCompileShaderFromFile(shadername, type, "main");
}

IDxcBlob** D3D12Shader::GetCurrentBlob(EShaderType::Type type)
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

std::wstring GetLevel()
{
	return L"_6_1";
}

std::wstring GetComplieTarget(EShaderType::Type t)
{
	switch (t)
	{
	case EShaderType::SHADER_COMPUTE:
		return L"cs" + GetLevel();
	case EShaderType::SHADER_VERTEX:
		return L"vs" + GetLevel();
	case EShaderType::SHADER_FRAGMENT:
		return L"ps" + GetLevel();
	case EShaderType::SHADER_GEOMETRY:
		return L"gs" + GetLevel();
	}
	return L"";
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
		ShaderReflection::GatherRSBinds(mBlolbs.GetBlob(ShaderType), GeneratedParams, IsCompute);
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
	IDxcBlobEncoding* pErrorBlob = NULL;
	HRESULT hr = S_OK;

	std::vector<LPCWSTR> arguments;
	if (ShaderComplier::Get()->ShouldBuildDebugShaders())
	{
		//compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_ALL_RESOURCES_BOUND;
		arguments.push_back(L"/Zi");
		arguments.push_back(L"/Od");
	}
	else
	{
		//	compileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_ALL_RESOURCES_BOUND | D3DCOMPILE_ENABLE_STRICTNESS /*| D3DCOMPILE_WARNINGS_ARE_ERRORS*/;
		arguments.push_back(L"/O3");
		arguments.push_back(L"/Ges");
	}


	IDxcCompiler* complier = nullptr;
	DxcCreateInstance(CLSID_DxcCompiler, __uuidof(IDxcCompiler), (void **)&complier);
	IDxcOperationResult* R;

	DxcDefine* defs = ParseDefinesDXC();
	IDxcLibrary *pLibrary;
	IDxcBlobEncoding *pSource;
	DxcCreateInstance(CLSID_DxcLibrary, __uuidof(IDxcLibrary), (void **)&pLibrary);
	pLibrary->CreateBlobWithEncodingFromPinned(ShaderData.c_str(), ShaderData.size(), CP_UTF8, &pSource);
	hr = complier->Compile(pSource, StringUtils::ConvertStringToWide(shadername).c_str(), StringUtils::ConvertStringToWide(Entrypoint).c_str(), GetComplieTarget(ShaderType).c_str(),
		arguments.data(), arguments.size(), defs, Defines.size(), nullptr, &R);
	R->GetResult(GetCurrentBlob(ShaderType));
	R->GetErrorBuffer(&pErrorBlob);
	R->GetStatus(&hr);

	if (!ShaderComplier::Get()->ShouldBuildDebugShaders())
	{
		//		StripD3DShader(GetCurrentBlob(ShaderType));
	}
	if (pErrorBlob)
	{
		std::string Log = "Shader Compile Output: ";
		Log.append(name);
		Log.append("\n");
		IDxcBlobEncoding *pPrintBlob16;
		// We can use the library to get our preferred encoding.
		pLibrary->GetBlobAsUtf8(pErrorBlob, &pPrintBlob16);
		std::string S = std::string((char*)pErrorBlob->GetBufferPointer(), (int)pPrintBlob16->GetBufferSize());
		Log.append(S);
		pPrintBlob16->Release();
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
			if (S.length() > 0)
			{
				Log::LogMessage(Log, Log::Severity::Warning);
			}
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
	ShaderReflection::GatherRSBinds(mBlolbs.GetBlob(ShaderType), GeneratedParams, IsCompute);
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

void WriteBlobToHandle(_In_opt_ IDxcBlob *pBlob, _In_ HANDLE hFile, _In_opt_ LPCWSTR pFileName)
{
	if (pBlob == nullptr)
	{
		return;
	}

	DWORD written;
	if (FALSE == WriteFile(hFile, pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(), &written, nullptr))
	{
		//IFT_Data(HRESULT_FROM_WIN32(GetLastError()), pFileName);
	}
}
void WriteBlobToFile(_In_opt_ IDxcBlob *pBlob, _In_ LPCWSTR pFileName)
{
	if (pBlob == nullptr)
	{
		return;
	}

	CHandle file(CreateFileW(pFileName, GENERIC_WRITE, FILE_SHARE_READ, nullptr,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr));
	if (file == INVALID_HANDLE_VALUE)
	{
		//IFT_Data(HRESULT_FROM_WIN32(GetLastError()), pFileName);
	}
	WriteBlobToHandle(pBlob, file, pFileName);
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
void ReadFileIntoBlob(LPCWSTR pFileName, IDxcBlobEncoding **ppBlobEncoding)
{
	IDxcLibrary* library;
	DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library));
	LogEnsure(library->CreateBlobFromFile(pFileName, nullptr, ppBlobEncoding) == S_OK);
	//ReadFileIntoPartContent(),
}

bool D3D12Shader::TryLoadCachedShader(const std::string& Name, IDxcBlob** Blob, const std::string & InstanceHash, EShaderType::Type type)
{
	if (!CacheBlobs)
	{
		return false;
	}
	const std::string FullShaderName = GetShaderNamestr(Name, InstanceHash, type);
	std::string ShaderPath = AssetManager::GetDDCPath() + "Shaders\\" + FullShaderName;
#if BUILD_PACKAGE
	ensureFatalMsgf(FileUtils::File_ExistsTest(ShaderPath), "Missing shader: " + GetShaderNamestr(Name, InstanceHash, type));
	ReadFileIntoBlob(StringUtils::ConvertStringToWide(ShaderPath).c_str(), (IDxcBlobEncoding**)Blob);
	return true;
#else	
	if (FileUtils::File_ExistsTest(ShaderPath) && CompareCachedShaderBlobWithSRC(Name, FullShaderName))
	{
		ReadFileIntoBlob(StringUtils::ConvertStringToWide(ShaderPath).c_str(), (IDxcBlobEncoding**)Blob);
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
		WriteBlobToFile(*GetCurrentBlob(type), StringUtils::ConvertStringToWide(DDcShaderPath + GetShaderNamestr(shadername, GetShaderInstanceHash(), type)).c_str());
	}
}

D3D12_SHADER_BYTECODE D3D12Shader::GetByteCode(IDxcBlob* b)
{
	return D3D12_SHADER_BYTECODE{ b->GetBufferPointer(), b->GetBufferSize() };
}

void D3D12Shader::CreateComputePipelineShader(D3D12PipeLineStateObject* output, D3D12_INPUT_ELEMENT_DESC* inputDisc, int DescCount, ShaderBlobs* blobs, const RHIPipeLineStateDesc& Depthtest,
	DeviceContext* context)
{
	SCOPE_STARTUP_COUNTER("Create Compute PSO");
	D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
	computePsoDesc.pRootSignature = output->RootSig;
	computePsoDesc.CS = GetByteCode(blobs->csBlob);
	computePsoDesc.NodeMask = context->GetNodeMask();
	ThrowIfFailed(((D3D12DeviceContext*)context)->GetDevice()->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&output->PSO)));

}

void D3D12Shader::CreatePipelineShader(D3D12PipeLineStateObject* output, D3D12_INPUT_ELEMENT_DESC* inputDisc, int DescCount, ShaderBlobs* blobs, const RHIPipeLineStateDesc& PSODesc,
	DeviceContext* context)
{
	SCOPE_STARTUP_COUNTER("Create PSO");
	ensure(blobs->vsBlob != nullptr);
	if (context == nullptr)
	{
		context = RHI::GetDeviceContext(0);
	}
	CD3DX12_PIPELINE_STATE_STREAM1 Stream;
	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = { 0 };
	psoDesc.InputLayout.pInputElementDescs = inputDisc;
	psoDesc.InputLayout.NumElements = DescCount;
	psoDesc.pRootSignature = output->RootSig;
	psoDesc.VS = GetByteCode(blobs->vsBlob);
	if (blobs->fsBlob != nullptr)
	{
		psoDesc.PS = GetByteCode(blobs->fsBlob);
	}
	if (blobs->gsBlob != nullptr)
	{
		psoDesc.GS = GetByteCode(blobs->gsBlob);
	}
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	if (PSODesc.RasterMode == PRIMITIVE_TOPOLOGY_TYPE::PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
	{
		if (context->GetDeviceIndex() == 0)
		{
			//Not fast raster, force consistent raster points 
			//#Cull useful for render culling?
			//psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON;
		}
	}
	psoDesc.RasterizerState.CullMode = PSODesc.Cull ? D3D12_CULL_MODE_BACK : D3D12_CULL_MODE_NONE;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.BlendState.AlphaToCoverageEnable = PSODesc.BlendState.AlphaToCoverageEnable;
	psoDesc.BlendState.IndependentBlendEnable = PSODesc.BlendState.IndependentBlendEnable;
	if (PSODesc.Blending)
	{
		psoDesc.BlendState.RenderTarget[0].BlendEnable = true;

		psoDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;

		if (PSODesc.Mode == Full)
		{
			psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
			psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND::D3D12_BLEND_SRC_ALPHA;
			psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND::D3D12_BLEND_INV_SRC_ALPHA;
			psoDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
			psoDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;

		}
		else if (PSODesc.Mode == Text)
		{
			psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND::D3D12_BLEND_SRC_ALPHA;
			psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND::D3D12_BLEND_INV_SRC_ALPHA;
		}
	}
	else
	{
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	}
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = PSODesc.DepthStencilState.DepthEnable;
	psoDesc.DepthStencilState.DepthFunc = (D3D12_COMPARISON_FUNC)PSODesc.DepthCompareFunction;
	psoDesc.DepthStencilState.DepthWriteMask = PSODesc.DepthStencilState.DepthWrite ? D3D12_DEPTH_WRITE_MASK::D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK::D3D12_DEPTH_WRITE_MASK_ZERO;
	psoDesc.DepthStencilState.StencilEnable = false;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = (D3D12_PRIMITIVE_TOPOLOGY_TYPE)PSODesc.RasterMode;
	psoDesc.NumRenderTargets = PSODesc.RenderTargetDesc.NumRenderTargets;
	psoDesc.SampleDesc.Count = PSODesc.SampleCount;
	psoDesc.NodeMask = context->GetNodeMask();

	for (int i = 0; i < 8; i++)
	{
		psoDesc.RTVFormats[i] = D3D12Helpers::ConvertFormat(PSODesc.RenderTargetDesc.RTVFormats[i]);
	}
	psoDesc.DSVFormat = D3D12Helpers::ConvertFormat(PSODesc.RenderTargetDesc.DSVFormat);

	if (((D3D12DeviceContext*)context)->GetDevice2() != nullptr)
	{
		Stream = CD3DX12_PIPELINE_STATE_STREAM1(psoDesc);
		D3D12_VIEW_INSTANCE_LOCATION* Loc = nullptr;
		if (PSODesc.ViewInstancing.Active)
		{
			ensure(context->GetCaps().SupportsViewInstancing);
			Loc = new D3D12_VIEW_INSTANCE_LOCATION[4];
			for (int i = 0; i < PSODesc.ViewInstancing.Instances; i++)
			{
				Loc[i].RenderTargetArrayIndex = 0;
				Loc[i].ViewportArrayIndex = 0;
			}
			CD3DX12_VIEW_INSTANCING_DESC D(PSODesc.ViewInstancing.Instances, &Loc[0], D3D12_VIEW_INSTANCING_FLAG_NONE);
			Stream.ViewInstancingDesc = D;
		}
		D3D12_PIPELINE_STATE_STREAM_DESC MyPipelineState;
		MyPipelineState.SizeInBytes = sizeof(Stream);
		MyPipelineState.pPipelineStateSubobjectStream = &Stream;
		ThrowIfFailed(((D3D12DeviceContext*)context)->GetDevice2()->CreatePipelineState(&MyPipelineState, IID_PPV_ARGS(&output->PSO)));
		delete[] Loc;
	}
	else
	{
		ThrowIfFailed(((D3D12DeviceContext*)context)->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&output->PSO)));
	}

}

D3D12Shader::ShaderBlobs * D3D12Shader::GetShaderBlobs()
{
	return &mBlolbs;
}

void D3D12Shader::Init()
{}

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

void D3D12Shader::CreateRootSig(D3D12PipeLineStateObject* output, std::vector<ShaderParameter> Params, DeviceContext* context, bool compute, std::vector<RHISamplerDesc> samplers)
{
	REF_CHECK(output->RootSig);
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
		if (Params[i].Type == ShaderParamType::SRV)
		{
			RangeNumber++;
		}
#if UAVRANGES
		else if (Params[i].Type == ShaderParamType::UAV)
		{
			RangeNumber++;
		}
#endif
	}
	D3D12_SHADER_VISIBILITY BaseSRVVis = D3D12_SHADER_VISIBILITY_ALL;
	if (compute)
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
		if (Params[i].Type == ShaderParamType::SRV)
		{
			ranges[Params[i].SignitureSlot].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, Params[i].NumDescriptors, Params[i].RegisterSlot, Params[i].RegisterSpace, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, 0);
			rootParameters[Params[i].SignitureSlot].InitAsDescriptorTable(1, &ranges[Params[i].SignitureSlot], compute ? BaseSRVVis : (D3D12_SHADER_VISIBILITY)Params[i].Visiblity);
		}
		else if (Params[i].Type == ShaderParamType::CBV)
		{
			rootParameters[Params[i].SignitureSlot].InitAsConstantBufferView(Params[i].RegisterSlot, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
		}
		else if (Params[i].Type == ShaderParamType::UAV)
		{
#if !UAVRANGES
			rootParameters[Params[i].SignitureSlot].InitAsUnorderedAccessView(Params[i].RegisterSlot, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
#else
			ranges[Params[i].SignitureSlot].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, Params[i].NumDescriptors, Params[i].RegisterSlot, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, 0);
			rootParameters[Params[i].SignitureSlot].InitAsDescriptorTable(1, &ranges[Params[i].SignitureSlot], D3D12_SHADER_VISIBILITY_ALL);
#endif
		}
		else if (Params[i].Type == ShaderParamType::RootConstant)
		{
			rootParameters[Params[i].SignitureSlot].InitAsConstants(Params[i].NumDescriptors, Params[i].RegisterSlot, Params[i].RegisterSpace, (D3D12_SHADER_VISIBILITY)Params[i].Visiblity);
		}
	}
	//#RHI: Samplers
#if 0
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
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	sampler.ShaderRegister = 2;
	sampler.RegisterSpace = 0;
	samplers[2] = sampler;
#else
	D3D12_STATIC_SAMPLER_DESC* Samplers = ConvertSamplers(samplers);

#endif
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc((UINT)Params.size(), rootParameters, samplers.size(), &Samplers[0], D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ID3DBlob* signature = nullptr;
	ID3DBlob* pErrorBlob = nullptr;
	HRESULT hr = (D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, featureData.HighestVersion, &signature, &pErrorBlob));
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
	ThrowIfFailed(((D3D12DeviceContext*)context)->GetDevice()->CreateRootSignature(context->GetNodeMask(), signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&output->RootSig)));

	output->RootSig->SetName(StringUtils::ConvertStringToWide(GetUniqueName(Params)).c_str());
	delete[] Samplers;
}
const std::string D3D12Shader::GetUniqueName(std::vector<ShaderParameter>& Params)
{
	std::string output = "Root sig Length = ";;
	output.append(std::to_string(Params.size()));
	for (ShaderParameter sp : Params)
	{
		output += "T";
		switch (sp.Type)
		{
		case ShaderParamType::RootConstant:
			output += "RC";
		case ShaderParamType::CBV:
			output += "CBV";
			break;
		case ShaderParamType::SRV:
			output += "SRV";
			break;
		case ShaderParamType::UAV:
			output += "UAV";
			break;
		default:
			break;
		}
		output += ", ";
	}
	return output;
}

IDxcBlob * D3D12Shader::ShaderBlobs::GetBlob(EShaderType::Type t)
{
	switch (t)
	{
	case EShaderType::SHADER_VERTEX:
		return vsBlob;
	case EShaderType::SHADER_FRAGMENT:
		return fsBlob;
	case EShaderType::SHADER_GEOMETRY:
		return gsBlob;
	case EShaderType::SHADER_COMPUTE:
		return csBlob;
	}
	return nullptr;
}


D3D12_STATIC_SAMPLER_DESC* D3D12Shader::ConvertSamplers(std::vector<RHISamplerDesc>& samplers)
{
	D3D12_STATIC_SAMPLER_DESC * out = new D3D12_STATIC_SAMPLER_DESC[samplers.size()];
	for (int i = 0; i < samplers.size(); i++)
	{
		RHISamplerDesc* Ins = &samplers[i];
		out[i].AddressU = (D3D12_TEXTURE_ADDRESS_MODE)Ins->UAddressMode;
		out[i].AddressV = (D3D12_TEXTURE_ADDRESS_MODE)Ins->VAddressMode;
		out[i].AddressW = (D3D12_TEXTURE_ADDRESS_MODE)Ins->WAddressMode;
		out[i].Filter = (D3D12_FILTER)Ins->FilterMode;
		out[i].MaxAnisotropy = Ins->MaxAnisotropy;
		out[i].ShaderRegister = Ins->ShaderRegister;

		out[i].MipLODBias = 0;
		out[i].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		out[i].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		out[i].MinLOD = 0.0f;
		out[i].MaxLOD = D3D12_FLOAT32_MAX;
		out[i].RegisterSpace = 0;
		out[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	}
	return out;
}