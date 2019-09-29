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
#include "RHI/ShaderPreProcessor.h"


static ConsoleVariable NoShaderCache("NoShaderCache", 0, ECVarType::LaunchOnly);
#if !BUILD_SHIPPING
D3D12Shader::ShaderStats D3D12Shader::stats = D3D12Shader::ShaderStats();
#endif
D3D12Shader::D3D12Shader(DeviceContext* Device)
{
	CurrentDevice = D3D12RHI::DXConv(Device);
	//	NoShaderCache.SetValue(true);
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
#if USE_DIXL

LPCWSTR GetCopyStr(std::string data)
{
	std::wstring t = StringUtils::ConvertStringToWide(data);
	wchar_t* Data = new wchar_t[t.size() + 1];
	t.copy(Data, t.size());
	Data[t.size()] = L'\0';
	return Data;
}


DxcDefine* D3D12Shader::ParseDefines()
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
#else
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

#endif


EShaderError::Type D3D12Shader::AttachAndCompileShaderFromFile(const char * shadername, EShaderType::Type type)
{
	return AttachAndCompileShaderFromFile(shadername, type, "main");
}

ShaderBlob** D3D12Shader::GetCurrentBlob(EShaderType::Type type)
{
	switch (type)
	{
		case EShaderType::SHADER_VERTEX:
			return &mBlolbs.vsBlob;
		case EShaderType::SHADER_FRAGMENT:
			return &mBlolbs.fsBlob;
		case EShaderType::SHADER_GEOMETRY:
			return &mBlolbs.gsBlob;
		case EShaderType::SHADER_COMPUTE:
			return &mBlolbs.csBlob;
		case EShaderType::SHADER_RT_LIB:
			return &mBlolbs.RTLibBlob;
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


std::wstring ConvertToLevelString(D3D_SHADER_MODEL SM)
{
	switch (SM)
	{
		case D3D_SHADER_MODEL_5_1:
#if USE_DIXL
			return L"_6_0";//dxil does not support 5_1 profiles
#else
			return L"_5_1";//dxil does not support 5_1 profiles
#endif
		case D3D_SHADER_MODEL_6_0:
			return L"_6_0";
		case D3D_SHADER_MODEL_6_1:
			return L"_6_1";
		case D3D_SHADER_MODEL_6_2:
			return L"_6_2";
#if WIN10_1809
		case D3D_SHADER_MODEL_6_3:
			return L"_6_3";
		case D3D_SHADER_MODEL_6_4:
			return L"_6_4";
#endif
	}
	return L"BAD!";
}
std::wstring D3D12Shader::GetShaderModelString(D3D_SHADER_MODEL Clamp)
{
	D3D12DeviceContext* Con = D3D12RHI::DXConv(RHI::GetDefaultDevice());
	D3D_SHADER_MODEL SM = Con->GetShaderModel();
#if !USE_DIXL
	SM = D3D_SHADER_MODEL_5_1;
#endif
	if (SM > Clamp)
	{
		SM = Clamp;
	}
	return ConvertToLevelString(SM);
}

std::wstring D3D12Shader::GetComplieTarget(EShaderType::Type t)
{
#if WIN10_1809
	const D3D_SHADER_MODEL ClampSm = D3D_SHADER_MODEL_6_3;
#else
	const D3D_SHADER_MODEL ClampSm = D3D_SHADER_MODEL_5_1;
#endif
	switch (t)
	{
		case EShaderType::SHADER_COMPUTE:
			return L"cs" + GetShaderModelString(ClampSm);
		case EShaderType::SHADER_VERTEX:
			return L"vs" + GetShaderModelString(ClampSm);
		case EShaderType::SHADER_FRAGMENT:
			//Currently there is no PS_6_4 target
			return L"ps" + GetShaderModelString(ClampSm);
		case EShaderType::SHADER_GEOMETRY:
			return L"gs" + GetShaderModelString(ClampSm);
#if WIN10_1809
		case EShaderType::SHADER_RT_LIB:
			return L"lib" + GetShaderModelString(ClampSm);
#endif
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
		ShaderReflection::GatherRSBinds(mBlolbs.GetBlob(ShaderType), ShaderType, GeneratedParams, IsCompute);
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
	if (ShaderData.length() == 0)
	{
		//#TODO: delete CSO		
		__debugbreak();
		return EShaderError::SHADER_ERROR_NOFILE;
	}
	HRESULT hr = S_OK;
#if USE_DIXL
	IDxcBlobEncoding* pErrorBlob = NULL;
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

	DxcDefine* defs = ParseDefines();
	IDxcLibrary *pLibrary;
	IDxcBlobEncoding *pSource;
	DxcCreateInstance(CLSID_DxcLibrary, __uuidof(IDxcLibrary), (void **)&pLibrary);
	pLibrary->CreateBlobWithEncodingFromPinned(ShaderData.c_str(), ShaderData.size(), CP_UTF8, &pSource);

	hr = complier->Compile(pSource, StringUtils::ConvertStringToWide(shadername).c_str(), StringUtils::ConvertStringToWide(Entrypoint).c_str(), GetComplieTarget(ShaderType).c_str(),
		arguments.data(), arguments.size(), defs, Defines.size(), nullptr, &R);
	R->GetResult(GetCurrentBlob(ShaderType));
	R->GetErrorBuffer(&pErrorBlob);
	R->GetStatus(&hr);
#else
	ID3DBlob* pErrorBlob = NULL;
	UINT  compileFlags = 0;
	if (ShaderComplier::Get()->ShouldBuildDebugShaders())
	{
		compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_ALL_RESOURCES_BOUND;
	}
	else
	{
		compileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_ALL_RESOURCES_BOUND | D3DCOMPILE_ENABLE_STRICTNESS /*| D3DCOMPILE_WARNINGS_ARE_ERRORS*/;
	}
	D3D_SHADER_MACRO* defines = ParseDefines();
	hr = D3DCompile(ShaderData.c_str(), ShaderData.size(), shadername, defines, nullptr, Entrypoint, StringUtils::ConvertWideToString(GetComplieTarget(ShaderType)).c_str(), compileFlags, 0, GetCurrentBlob(ShaderType), &pErrorBlob);
#endif
	if (!ShaderComplier::Get()->ShouldBuildDebugShaders())
	{
		//		StripD3DShader(GetCurrentBlob(ShaderType));
	}
	if (pErrorBlob)
	{
		std::string Log = "Shader Compile Output: ";
		Log.append(name);
		Log.append("\n");
#if USE_DIXL
		IDxcBlobEncoding *pPrintBlob16;
		// We can use the library to get our preferred encoding.
		pLibrary->GetBlobAsUtf8(pErrorBlob, &pPrintBlob16);
		std::string S = std::string((char*)pErrorBlob->GetBufferPointer(), (int)pPrintBlob16->GetBufferSize());
		Log.append(S);
		pPrintBlob16->Release();
#else
		std::string S = reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer());
		Log.append(S);
#endif
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
	ShaderReflection::GatherRSBinds(mBlolbs.GetBlob(ShaderType), ShaderType, GeneratedParams, IsCompute);
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
#if WIN10_1809
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
#endif
const std::string D3D12Shader::GetShaderNamestr(const std::string & Shadername, const std::string & InstanceHash, EShaderType::Type type)
{
	std::string OutputName = Shadername;
	OutputName += "_" + std::to_string((int)type);
	OutputName += "_" + InstanceHash;
	if (ShaderComplier::Get()->ShouldBuildDebugShaders())
	{
		OutputName += "_D";
	}
#if USE_DIXL
	OutputName += "_DIXL";
#else
	OutputName += "_D3D";
#endif
	OutputName += ".cso";
	return OutputName;
}
#if WIN10_1809
void ReadFileIntoBlob(LPCWSTR pFileName, IDxcBlobEncoding **ppBlobEncoding)
{
	IDxcLibrary* library;
	DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library));
	LogEnsure(library->CreateBlobFromFile(pFileName, nullptr, ppBlobEncoding) == S_OK);
	//ReadFileIntoPartContent(),
}
#endif
bool D3D12Shader::TryLoadCachedShader(const std::string& Name, ShaderBlob** Blob, const std::string & InstanceHash, EShaderType::Type type)
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
	if (FileUtils::File_ExistsTest(ShaderPath) && ShaderPreProcessor::CheckCSOValid(Name, FullShaderName))
	{
#if USE_DIXL
		ReadFileIntoBlob(StringUtils::ConvertStringToWide(ShaderPath).c_str(), (IDxcBlobEncoding**)Blob);
#else
		ThrowIfFailed(D3DReadFileToBlob(StringUtils::ConvertStringToWide(ShaderPath).c_str(), Blob));
#endif
		return true;
	}
	Log::LogMessage("Recompile triggered for " + Name);
	return false;
#endif
}

void D3D12Shader::WriteBlobs(const std::string & shadername, EShaderType::Type type)
{
	if (CacheBlobs)
	{
		const std::string DDcShaderPath = AssetManager::GetDDCPath() + "Shaders\\";
		FileUtils::CreateDirectoriesToFullPath(DDcShaderPath + shadername + ".");
#if USE_DIXL
		WriteBlobToFile(*GetCurrentBlob(type), StringUtils::ConvertStringToWide(DDcShaderPath + GetShaderNamestr(shadername, GetShaderInstanceHash(), type)).c_str());
#else
		ThrowIfFailed(D3DWriteBlobToFile(*GetCurrentBlob(type), StringUtils::ConvertStringToWide(DDcShaderPath + GetShaderNamestr(shadername, GetShaderInstanceHash(), type)).c_str(), true));
#endif
	}
}

D3D12_SHADER_BYTECODE D3D12Shader::GetByteCode(ShaderBlob* b)
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
	ThrowIfFailed(D3D12RHI::DXConv(context)->GetDevice()->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&output->PSO)));

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

	if (D3D12RHI::DXConv(context)->GetDevice2() != nullptr)
	{
		Stream = CD3DX12_PIPELINE_STATE_STREAM1(psoDesc);
		D3D12_VIEW_INSTANCE_LOCATION* Loc = nullptr;
		if (context->GetCaps().SupportsViewInstancing && PSODesc.ViewInstancing.Active)
		{
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
		}
		if (context->GetCaps().SupportsDepthBoundsTest && PSODesc.EnableDepthBoundsTest)
		{
			CD3DX12_DEPTH_STENCIL_DESC1 DepthDesc(psoDesc.DepthStencilState);
			DepthDesc.DepthBoundsTestEnable = true;
			Stream.DepthStencilState = DepthDesc;
		}
		D3D12_PIPELINE_STATE_STREAM_DESC MyPipelineState;
		MyPipelineState.SizeInBytes = sizeof(Stream);
		MyPipelineState.pPipelineStateSubobjectStream = &Stream;
		ThrowIfFailed(D3D12RHI::DXConv(context)->GetDevice2()->CreatePipelineState(&MyPipelineState, IID_PPV_ARGS(&output->PSO)));
		if (Loc != nullptr)
		{
			delete[] Loc;
		}
	}
	else
	{
		ThrowIfFailed(D3D12RHI::DXConv(context)->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&output->PSO)));
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

void D3D12Shader::CreateRootSig(ID3D12RootSignature ** output, std::vector<ShaderParameter> Params, DeviceContext * context, bool compute, std::vector<RHISamplerDesc> samplers,
	RootSignitureCreateInfo Info /*= RootSignitureCreateInfo()*/)
{
	//	REF_CHECK(output->RootSig);
	SCOPE_STARTUP_COUNTER("CreateRootSig");
	D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

	// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
	featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

	if (FAILED(D3D12RHI::DXConv(context)->GetDevice()->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
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
		if (Params[i].Type == ShaderParamType::SRV || Params[i].Type == ShaderParamType::Buffer)
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
		if (Params[i].Type == ShaderParamType::SRV || Params[i].Type == ShaderParamType::Buffer)
		{
			ranges[Params[i].SignitureSlot].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, Params[i].NumDescriptors, Params[i].RegisterSlot, Params[i].RegisterSpace, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, 0);
			rootParameters[Params[i].SignitureSlot].InitAsDescriptorTable(1, &ranges[Params[i].SignitureSlot], compute ? BaseSRVVis : (D3D12_SHADER_VISIBILITY)Params[i].Visiblity);
		}
		else if (Params[i].Type == ShaderParamType::CBV || Params[i].Type == ShaderParamType::Buffer)
		{
			rootParameters[Params[i].SignitureSlot].InitAsConstantBufferView(Params[i].RegisterSlot, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
		}
		else if (Params[i].Type == ShaderParamType::RootSRV)
		{
			rootParameters[Params[i].SignitureSlot].InitAsShaderResourceView(Params[i].RegisterSlot, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
		}
		else if (Params[i].Type == ShaderParamType::UAV)
		{
#if !UAVRANGES
			rootParameters[Params[i].SignitureSlot].InitAsUnorderedAccessView(Params[i].RegisterSlot, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, D3D12_SHADER_VISIBILITY_ALL);
#else
			ranges[Params[i].SignitureSlot].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, Params[i].NumDescriptors, Params[i].RegisterSlot, 0, /*D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC*/ D3D12_DESCRIPTOR_RANGE_FLAG_NONE, 0);
			rootParameters[Params[i].SignitureSlot].InitAsDescriptorTable(1, &ranges[Params[i].SignitureSlot], D3D12_SHADER_VISIBILITY_ALL);
#endif
		}
		else if (Params[i].Type == ShaderParamType::RootConstant)
		{
			rootParameters[Params[i].SignitureSlot].InitAsConstants(Params[i].NumDescriptors, Params[i].RegisterSlot, Params[i].RegisterSpace, (D3D12_SHADER_VISIBILITY)Params[i].Visiblity);
		}
	}
	//#RHI: Samplers

	D3D12_STATIC_SAMPLER_DESC* Samplers = ConvertSamplers(samplers);
	D3D12_ROOT_SIGNATURE_FLAGS RsFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
#if WIN10_1809
	if (Info.IsLocalSig)
	{
		RsFlags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
	}
#endif
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc((UINT)Params.size(), rootParameters, samplers.size(), &Samplers[0], RsFlags);

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
	ThrowIfFailed(D3D12RHI::DXConv(context)->GetDevice()->CreateRootSignature(context->GetNodeMask(), signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(output)));

	(*output)->SetName(StringUtils::ConvertStringToWide(GetUniqueName(Params)).c_str());
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

ShaderBlob * D3D12Shader::ShaderBlobs::GetBlob(EShaderType::Type t)
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
		case EShaderType::SHADER_RT_LIB:
			return RTLibBlob;
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