#include "D3D12Shader.h"
#include "Core/Assets/AssetManager.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Platform/ConsoleVariable.h"
#include "Core/Utils/FileUtils.h"
#include "D3D12DeviceContext.h"
#include "D3D12RHI.h"

#include "D3D12CommandList.h"
#include "ShaderReflection.h"
//#include <atlbase.h>
#include "RHI/ShaderPreProcessor.h"
#ifdef BUILD_NVAPI
#include "nvapi.h"
#endif
#include "Packaging/Cooker.h"

#ifdef PLATFORM_WINDOWS
#include <fileapi.h>
#include <atlbase.h>
#endif
#include "RHI/ShaderComplierModule.h"
#include "Rendering/Core/ShaderCache.h"


static ConsoleVariable ShaderCompileStats("ShaderStats", 0, ECVarType::LaunchOnly);
#if !BUILD_SHIPPING
D3D12Shader::ShaderStats D3D12Shader::stats = D3D12Shader::ShaderStats();
#endif
D3D12Shader::D3D12Shader(DeviceContext* Device)
	:ShaderProgramBase(Device)
{
	ShaderCompileStats.SetValue(true);
	CurrentDevice = D3D12RHI::DXConv(Device);
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
glm::ivec3 D3D12Shader::GetComputeThreadSize() const
{
	return ComputeThreadSize;
}


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

void D3D12Shader::ReportStats(ShaderSourceFile* ShaderData)
{
	if (ShaderCompileStats.GetBoolValue())
	{
		std::string Msg = "Shader compile Stats\n";
		float time = PerfManager::Get()->EndSingleActionTimer("D3D12Shader::AttachAndCompileShaderFromFile");
		PerfManager::Get()->FlushSingleActionTimer("D3D12Shader::AttachAndCompileShaderFromFile");
		Msg += "Took: " + StringUtils::ToString(time) + "ms\n";
		if (ShaderData != nullptr)
		{
			Msg += "Lines: " + std::to_string(ShaderData->LineCount) + " (Length: " + std::to_string(ShaderData->Source.length()) + ")\n";
		}
		Msg += "Instruction Count: " + std::to_string(InstructionCount) + "";
		Log::LogMessage(Msg);
	}
}

EShaderError::Type D3D12Shader::AttachAndCompileShaderFromFile(const char * shadername, EShaderType::Type ShaderType, const char * Entrypoint)
{
#if !BUILD_SHIPPING
	stats.TotalShaderCount++;
#endif
	ShaderComplieItem*  item = new ShaderComplieItem();
	item->ShaderName = shadername;
	item->Defines = Defines;
	item->Data = AssetManager::Get()->LoadFileWithInclude(item->ShaderName + ".hlsl");
	item->Stage = ShaderType;
	item->ShaderModel = RHI::GetDefaultDevice()->GetCaps().HighestModel;
	item->EntryPoint = Entrypoint;
	ShaderByteCodeBlob* blob = ShaderCache::GetShader(item);
	if (blob == nullptr)
	{
		return EShaderError::SHADER_ERROR_COMPILE;
	}
	IDxcLibrary* library;
	DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library));
	HRESULT r = library->CreateBlobWithEncodingFromPinned(blob->ByteCode, blob->Length, 0, (IDxcBlobEncoding**)GetCurrentBlob(ShaderType));
	ensure(r == S_OK);
	const std::string FullShaderName = ShaderCache::GetShaderNamestr(shadername, ShaderCache::GetShaderInstanceHash(item), ShaderType);
	IDxcBlob* RelfectionBlob = mBlolbs.GetBlob(ShaderType);
	if (item->ReflectionBlob != nullptr)
	{
#ifndef PLATFORM_WINDOWS
		HRESULT r = library->CreateBlobWithEncodingFromPinned(item->ReflectionBlob->ByteCode, item->ReflectionBlob->Length, 0, (IDxcBlobEncoding**)&RelfectionBlob);
		ensure(r == S_OK);
#endif
}
	ShaderReflection::GatherRSBinds(RelfectionBlob, ShaderType, GeneratedParams, IsCompute, item->Data, this);
	if (item->CacheHit)
	{
#if !BUILD_SHIPPING
		stats.ShaderLoadFromCacheCount++;
#endif
		return EShaderError::SHADER_ERROR_NONE;
	}
#if !BUILD_SHIPPING
	stats.ShaderComplieCount++;
#endif
	ReportStats(item->Data);
	if (ShaderType == EShaderType::SHADER_COMPUTE)
	{
		uint ThreadCount = ComputeThreadSize.x + ComputeThreadSize.y + ComputeThreadSize.z;
		if (ThreadCount < 32)
		{
			AD_WARN("'" + item->ShaderName + "' Has Low Thread count (" + std::to_string(ThreadCount) + ") this will cause under utilization on NVidia hardware(32)");
		}
		else if (ThreadCount < 64)
		{
			AD_WARN("'" + item->ShaderName + "' Has Low Thread count  (" + std::to_string(ThreadCount) + ") this will cause under utilization on AMD hardware(64)");
		}
	}
	return EShaderError::SHADER_ERROR_NONE;
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
	ThrowIfFailed(D3D12RHI::DXConv(context)->GetDevice()->CreateComputePipelineState(&computePsoDesc, ID_PASS(&output->PSO)));
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
	//CD3DX12_PIPELINE_STATE_STREAM1 on win64
#ifdef PLATFORM_WINDOWS
	CD3DX12_PIPELINE_STATE_STREAM1 Stream;
#else
	CD3DX12_PIPELINE_STATE_STREAM Stream;
#endif
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
		//if (D3D12RHI::DXConv(context)->GetFeatureData().FeatureData.TypedUAVLoadAdditionalFormats)

		psoDesc.RasterizerState.ConservativeRaster = PSODesc.RasterizerState.ConservativeRaster ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		if (psoDesc.RasterizerState.ConservativeRaster)
		{
			if (!context->GetCaps().SupportsConservativeRaster)
			{
				LogEnsure_Always("Device does not support Conservative Raster, Feature Disabled");
				psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
			}
		}
	}
	psoDesc.RasterizerState.CullMode = PSODesc.Cull ? D3D12_CULL_MODE_BACK : D3D12_CULL_MODE_NONE;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.BlendState.AlphaToCoverageEnable = PSODesc.BlendState.AlphaToCoverageEnable;
	psoDesc.BlendState.IndependentBlendEnable = PSODesc.BlendState.IndependentBlendEnable;
	if (PSODesc.BlendState.RenderTargetDescs[0].BlendEnable)
	{
		for (int i = 0; i < MRT_MAX; i++)
		{
			const RHIRender_Target_Blend_Desc* SRCD = &PSODesc.BlendState.RenderTargetDescs[i];
			D3D12_RENDER_TARGET_BLEND_DESC* Dst = &psoDesc.BlendState.RenderTarget[i];
			if (!SRCD->BlendEnable)
			{
				Dst->BlendEnable = false;
				continue;
			}
			Dst->BlendEnable = true;
			Dst->BlendOp = (D3D12_BLEND_OP)SRCD->BlendOp;
			Dst->SrcBlend = (D3D12_BLEND)SRCD->SrcBlend;
			Dst->DestBlend = (D3D12_BLEND)SRCD->DestBlend;
			Dst->BlendOpAlpha = (D3D12_BLEND_OP)SRCD->BlendOpAlpha;
			Dst->SrcBlendAlpha = (D3D12_BLEND)SRCD->SrcBlendAlpha;
			Dst->DestBlendAlpha = (D3D12_BLEND)SRCD->DestBlendAlpha;
			Dst->RenderTargetWriteMask = SRCD->RenderTargetWriteMask;
		}
	}
	else
	{
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	}
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = PSODesc.DepthStencilState.DepthEnable;
	psoDesc.DepthStencilState.DepthFunc = (D3D12_COMPARISON_FUNC)PSODesc.DepthStencilState.DepthCompareFunction;
	psoDesc.DepthStencilState.DepthWriteMask = PSODesc.DepthStencilState.DepthWrite ? D3D12_DEPTH_WRITE_MASK::D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK::D3D12_DEPTH_WRITE_MASK_ZERO;
	psoDesc.DepthStencilState.StencilEnable = PSODesc.DepthStencilState.StencilEnable;

	psoDesc.DepthStencilState.FrontFace.StencilDepthFailOp = (D3D12_STENCIL_OP)PSODesc.DepthStencilState.FrontFace.StencilDepthFailOp;
	psoDesc.DepthStencilState.FrontFace.StencilFailOp = (D3D12_STENCIL_OP)PSODesc.DepthStencilState.FrontFace.StencilFailOp;
	psoDesc.DepthStencilState.FrontFace.StencilPassOp = (D3D12_STENCIL_OP)PSODesc.DepthStencilState.FrontFace.StencilPassOp;
	psoDesc.DepthStencilState.FrontFace.StencilFunc = (D3D12_COMPARISON_FUNC)PSODesc.DepthStencilState.FrontFace.StencilFunc;

	psoDesc.DepthStencilState.BackFace.StencilDepthFailOp = (D3D12_STENCIL_OP)PSODesc.DepthStencilState.BackFace.StencilDepthFailOp;
	psoDesc.DepthStencilState.BackFace.StencilFailOp = (D3D12_STENCIL_OP)PSODesc.DepthStencilState.BackFace.StencilFailOp;
	psoDesc.DepthStencilState.BackFace.StencilPassOp = (D3D12_STENCIL_OP)PSODesc.DepthStencilState.BackFace.StencilPassOp;
	psoDesc.DepthStencilState.BackFace.StencilFunc = (D3D12_COMPARISON_FUNC)PSODesc.DepthStencilState.BackFace.StencilFunc;

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
#ifdef PLATFORM_WINDOWS
		Stream = CD3DX12_PIPELINE_STATE_STREAM1(psoDesc);
#else
		Stream = CD3DX12_PIPELINE_STATE_STREAM(psoDesc);
#endif
		D3D12_VIEW_INSTANCE_LOCATION* Loc = nullptr;
		if (PSODesc.ViewInstancing.Active)
		{
			bool FallBack = true;
			if (PSODesc.ViewInstancing.NV_UseSMP)
			{
				if (context->IsDeviceNVIDIA() && RHI::AllowIHVAcceleration())
				{
					if (PSODesc.ViewInstancing.Instances == 2)
					{
						//use SMT
						LogEnsureMsgf(false, "Accleration possible using SMP");
						//FallBack = false;
						////NvAPI_D3D12_SetSinglePassStereoMode(nullptr,2,0,0);
					}
				}
			}
			if (context->GetCaps().SupportsViewInstancing && FallBack)
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
		ThrowIfFailed(D3D12RHI::DXConv(context)->GetDevice2()->CreatePipelineState(&MyPipelineState, ID_PASS(&output->PSO)));
		if (Loc != nullptr)
		{
			delete[] Loc;
		}
	}
	else
	{
		ThrowIfFailed(D3D12RHI::DXConv(context)->GetDevice()->CreateGraphicsPipelineState(&psoDesc, ID_PASS(&output->PSO)));
	}
#if 0
	if (blobs->gsBlob != nullptr && RHI::AllowIHVAcceleration() && context->IsDeviceNVIDIA() && false)
	{
		std::vector<const NVAPI_D3D12_PSO_EXTENSION_DESC*> Extentions;
		NVAPI_D3D12_PSO_GEOMETRY_SHADER_DESC_V5* ext = new NVAPI_D3D12_PSO_GEOMETRY_SHADER_DESC_V5();
		ext->version = NV_GEOMETRY_SHADER_PSO_EXTENSION_DESC_VER;
		ext->psoExtension = NV_PSO_EXTENSION::NV_PSO_GEOMETRY_SHADER_EXTENSION;
		//ext->ConvertToFastGS = true;
		//ext->ForceFastGS = true;
		ext->baseVersion = NV_PSO_EXTENSION_DESC_VER;
		Extentions.push_back(ext);
		NvAPI_Status r = NvAPI_D3D12_CreateGraphicsPipelineState(D3D12RHI::DXConv(context)->GetDevice(), &psoDesc, Extentions.size(), Extentions.data(), &output->PSO);
		AD_Assert(r == NvAPI_Status::NVAPI_OK);
	}
#endif
}

D3D12Shader::ShaderBlobs * D3D12Shader::GetShaderBlobs()
{
	return &mBlolbs;
}

void D3D12Shader::Init()
{}

D3D12_INPUT_ELEMENT_DESC D3D12Shader::ConvertVertexFormat(VertexElementDESC* desc)
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
bool D3D12Shader::ParseVertexFormat(std::vector<VertexElementDESC> desc, D3D12_INPUT_ELEMENT_DESC ** Data, int * length)
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

	CD3DX12_DESCRIPTOR_RANGE1* ranges = nullptr;
	if (RangeNumber > 0)
	{
		ranges = new CD3DX12_DESCRIPTOR_RANGE1[Params.size()];
	}
	for (int i = 0; i < Params.size(); i++)
	{
		D3D12_SHADER_VISIBILITY ShaderVisible = (D3D12_SHADER_VISIBILITY)Params[i].Visiblity;
		if (compute)
		{
			ShaderVisible = D3D12_SHADER_VISIBILITY_ALL;
		}
		if (Params[i].Type == ShaderParamType::SRV || Params[i].Type == ShaderParamType::Buffer)
		{
			ranges[Params[i].SignitureSlot].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, Params[i].NumDescriptors, Params[i].RegisterSlot, Params[i].RegisterSpace, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, 0);
			rootParameters[Params[i].SignitureSlot].InitAsDescriptorTable(1, &ranges[Params[i].SignitureSlot], ShaderVisible);
		}
		else if (Params[i].Type == ShaderParamType::CBV || Params[i].Type == ShaderParamType::Buffer)
		{
			rootParameters[Params[i].SignitureSlot].InitAsConstantBufferView(Params[i].RegisterSlot, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, ShaderVisible);
		}
		else if (Params[i].Type == ShaderParamType::RootSRV)
		{
			rootParameters[Params[i].SignitureSlot].InitAsShaderResourceView(Params[i].RegisterSlot, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE, ShaderVisible);
		}
		else if (Params[i].Type == ShaderParamType::UAV)
		{
#if !UAVRANGES
			rootParameters[Params[i].SignitureSlot].InitAsUnorderedAccessView(Params[i].RegisterSlot, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE, ShaderVisible);
#else
			ranges[Params[i].SignitureSlot].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, Params[i].NumDescriptors, Params[i].RegisterSlot, 0, /*D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC*/ D3D12_DESCRIPTOR_RANGE_FLAG_NONE, 0);
			rootParameters[Params[i].SignitureSlot].InitAsDescriptorTable(1, &ranges[Params[i].SignitureSlot], ShaderVisible);
#endif
	}
		else if (Params[i].Type == ShaderParamType::RootConstant)
		{
			rootParameters[Params[i].SignitureSlot].InitAsConstants(Params[i].NumVariablesContained, Params[i].RegisterSlot, Params[i].RegisterSpace, ShaderVisible);
		}
}
	D3D12_STATIC_SAMPLER_DESC* Samplers = ConvertSamplers(samplers);
	D3D12_ROOT_SIGNATURE_FLAGS RsFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

#if WIN10_1809
	if (Info.IsLocalSig)
	{
		RsFlags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
	}
	if (Info.IsGlobalSig)
	{
		RsFlags = D3D12RHIConfig::RTRootSigExtraFlag;
	}
#endif
	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc((UINT)Params.size(), rootParameters, (UINT)samplers.size(), &Samplers[0], RsFlags);

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
	ThrowIfFailed(D3D12RHI::DXConv(context)->GetDevice()->CreateRootSignature(context->GetNodeMask(), signature->GetBufferPointer(), signature->GetBufferSize(), ID_PASS(output)));

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
		//out[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		out[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	}
	return out;
}