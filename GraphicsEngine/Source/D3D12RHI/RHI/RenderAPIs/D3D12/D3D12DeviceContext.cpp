#include <sdkddkver.h>
#include "D3D12DeviceContext.h"
#include "D3D12TimeManager.h"
#include "D3D12CommandList.h"
#include "D3D12RHI.h"
#include "Core/Performance/PerfManager.h"
#include "DescriptorHeapManager.h"
#include "D3D12QueryHeap.h"
#include "DXMemoryManager.h"
#include "Core/Utils/NVAPIManager.h"
#include "Core/Utils/StringUtil.h"
#include "DXGPUTextureStreamer.h"
#include "DescriptorHeap.h"
#include "DescriptorCache.h"

#include "GPUResource.h"

#if NAME_RHI_PRIMS
#define DEVICE_NAME_OBJECT(x) NameObject(x,L#x, this->GetDeviceIndex())
void NameObject(ID3D12Object* pObject, std::wstring name, int id)
{
	name.append(L"_");
	name.append(std::to_wstring(id));
	NAME_D3D12_SetName(pObject, name.c_str());
}
#else
#define DEVICE_NAME_OBJECT(x);
#endif

static ConsoleVariable EnableStablePower("StablePower", false, ECVarType::LaunchOnly, true);

D3D12DeviceContext::D3D12DeviceContext()
{}

D3D12DeviceContext::~D3D12DeviceContext()
{
	DestoryDevice();
	MemoryUtils::DeleteVector(Allocators);
	SafeDelete(MemoryManager);
	SafeRelease(m_MainCommandQueue);
	SafeDelete(TimeManager);
	SafeRHIRelease(GPUCopyList);
	SafeRHIRelease(InterGPUCopyList);
	SafeDelete(TimeStampHeap);
	SafeDelete(CopyTimeStampHeap);
	SafeRelease(m_SharedCopyCommandQueue);
	SafeRelease(m_ComputeCommandQueue);
	SafeRelease(m_CopyCommandQueue);
	SafeRelease(m_Device);
	SafeDelete(HeapManager);
#if SUPPORT_DXGI
	SafeRelease(pDXGIAdapter);
#endif
	/*if (pDXGIAdapter != nullptr)
	{
		pDXGIAdapter->UnregisterVideoMemoryBudgetChangeNotification(m_BudgetNotificationCookie);
	}*/
}

bool D3D12DeviceContext::DetectDriverDXR()
{
	//Not great but there isn't an API check.
	std::wstring Data = Adaptordesc.Description;
	std::wstring Strings[] = { L"10",L"Titan X",L"1660",L"Titan V" };
	for (std::wstring s : Strings)
	{
		if (StringUtils::Contains(Data, s))
		{
			return true;
		}
	}
	return false;
}

void D3D12DeviceContext::LogFeatureData(std::string name, bool value)
{
	Log::LogMessage("Device " + std::to_string(GetDeviceIndex()) + " " + name + " " + (value ? "true " : "false "));
}

void D3D12DeviceContext::CheckFeatures()
{
	//#DX12: validate the device capabilities 
	D3D12_FEATURE_DATA_D3D12_OPTIONS options = {};
	ThrowIfFailed(GetDevice()->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, reinterpret_cast<void*>(&options), sizeof(options)));

	DeviceFeatureData.ArchData.NodeIndex = 0;
	ThrowIfFailed(GetDevice()->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE1, reinterpret_cast<void*>(&DeviceFeatureData.ArchData), sizeof(DeviceFeatureData.ArchData)));
	if (DeviceFeatureData.ArchData.UMA)
	{
		GPUType = EGPUType::Intergrated;
	}
	else
	{
		GPUType = EGPUType::Dedicated;
	}
	if (m_Device->GetNodeCount() > 1)
	{
		GPUType = EGPUType::Dedicated_Linked;
	}
	if (Adaptordesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
	{
		GPUType = EGPUType::Software;
	}
	LogFeatureData("UMA", DeviceFeatureData.ArchData.UMA);
#if 0
	LogFeatureData("TileBasedRenderer", ARCHDAta.TileBasedRenderer);
	LogFeatureData("IsolatedMMU", ARCHDAta.IsolatedMMU);
#endif

	ReportData();

	ZeroMemory(&DeviceFeatureData.FeatureData, sizeof(DeviceFeatureData.FeatureData));
	HRESULT hr = m_Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &DeviceFeatureData.FeatureData, sizeof(DeviceFeatureData.FeatureData));
	if (SUCCEEDED(hr) && LogDeviceDebug)
	{
		LogTierData("Resource Binding", DeviceFeatureData.FeatureData.ResourceBindingTier);
		LogTierData("Resource Heap", DeviceFeatureData.FeatureData.ResourceHeapTier);
		LogTierData("Cross Node Sharing", DeviceFeatureData.FeatureData.CrossNodeSharingTier);
		LogTierData("Tiled Resources Tier", DeviceFeatureData.FeatureData.TiledResourcesTier);

		LogTierData("Min Precision Support", options.MinPrecisionSupport);
		Caps_Data.SupportTypedUAVLoads = DeviceFeatureData.FeatureData.TypedUAVLoadAdditionalFormats;
	}

	Caps_Data.SupportsConservativeRaster = DeviceFeatureData.FeatureData.ConservativeRasterizationTier >= D3D12_CONSERVATIVE_RASTERIZATION_TIER_1;
	ZeroMemory(&DeviceFeatureData.FeatureData1, sizeof(DeviceFeatureData.FeatureData1));
	hr = m_Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS1, &DeviceFeatureData.FeatureData1, sizeof(DeviceFeatureData.FeatureData1));
	if (SUCCEEDED(hr) && LogDeviceDebug)
	{
		LogDeviceData("Threads Per warp count: " + std::to_string(DeviceFeatureData.FeatureData1.WaveLaneCountMin));
	}
	ZeroMemory(&DeviceFeatureData.FeatureData2, sizeof(DeviceFeatureData.FeatureData2));
	hr = m_Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS2, &DeviceFeatureData.FeatureData2, sizeof(DeviceFeatureData.FeatureData2));
#if WIN10_1809
	ZeroMemory(&DeviceFeatureData.FeatureData5, sizeof(DeviceFeatureData.FeatureData5));
	hr = m_Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &DeviceFeatureData.FeatureData5, sizeof(DeviceFeatureData.FeatureData5));
	if (SUCCEEDED(hr))
	{
		if (LogDeviceDebug)
		{
			LogTierData("DXR", DeviceFeatureData.FeatureData5.RaytracingTier);
			LogTierData("Render Pass Driver", DeviceFeatureData.FeatureData5.RenderPassesTier);
		}
		//#DXR Detect driver RT
		if (DeviceFeatureData.FeatureData5.RaytracingTier)
		{
			Caps_Data.RTSupport = ERayTracingSupportType::Hardware;
			if (DetectDriverDXR())
			{
				Caps_Data.RTSupport = ERayTracingSupportType::DriverBased;
			}
		}
		else
		{
			Caps_Data.RTSupport = ERayTracingSupportType::None;
		}
		LogDeviceData("Ray tracing support mode is " + std::string(ERayTracingSupportType::ToString(Caps_Data.RTSupport)));
		SupportsCmdsList4 = true;
	}
	else
#endif
	{
		Log::LogMessage("System does not support DXR");
		Caps_Data.RTSupport = ERayTracingSupportType::None;
		SupportsCmdsList4 = false;
	}

	ZeroMemory(&DeviceFeatureData.FeatureData3, sizeof(DeviceFeatureData.FeatureData3));
	hr = m_Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS3, &DeviceFeatureData.FeatureData3, sizeof(DeviceFeatureData.FeatureData3));
	if (SUCCEEDED(hr))
	{
		if (LogDeviceDebug)
		{
			LogTierData("VIEW INSTANCING", DeviceFeatureData.FeatureData3.ViewInstancingTier);
		}
		Caps_Data.SupportsCopyTimeStamps = DeviceFeatureData.FeatureData3.CopyQueueTimestampQueriesSupported;
		Caps_Data.SupportsViewInstancing = (DeviceFeatureData.FeatureData3.ViewInstancingTier > D3D12_VIEW_INSTANCING_TIER_NOT_SUPPORTED);
	}

#if WIN10_1809
	const D3D_SHADER_MODEL MaxSM = D3D_SHADER_MODEL_6_4;
#else
	const D3D_SHADER_MODEL MaxSM = D3D_SHADER_MODEL_6_1;
#endif
	for (int i = MaxSM; i > D3D_SHADER_MODEL_5_1; i--)
	{
		D3D12_FEATURE_DATA_SHADER_MODEL  ShaderModelData = {};
		ShaderModelData.HighestShaderModel = (D3D_SHADER_MODEL)i;
		hr = m_Device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &ShaderModelData, sizeof(ShaderModelData));
		if (SUCCEEDED(hr))
		{
			HighestShaderModel = ShaderModelData.HighestShaderModel;
			break;
		}
	}
	//HighestShaderModel = D3D_SHADER_MODEL_5_1;
#if USE_DIXL
	switch (HighestShaderModel)
	{
	case D3D_SHADER_MODEL_5_1:
		Caps_Data.HighestModel = EShaderSupportModel::SM5;
		break;
	case D3D_SHADER_MODEL_6_0:
	case D3D_SHADER_MODEL_6_1:
#if WIN10_1809
	case D3D_SHADER_MODEL_6_2:
	case D3D_SHADER_MODEL_6_3:
	case D3D_SHADER_MODEL_6_4:
#endif
#if WIN10_1903 && defined(PLATFORM_WINDOWS)
	case D3D_SHADER_MODEL_6_5:
#endif
		Caps_Data.HighestModel = EShaderSupportModel::SM6;
		break;
	}
#else
	Log::LogMessage("Compiling Without DXIL complier, shader model is limited to SM5");
	Caps_Data.HighestModel = EShaderSupportModel::SM5;
#endif
	if (LogDeviceDebug)
	{
		LogDeviceData("Shader Model Support (DX12) " + D3D12Helpers::SMToString(HighestShaderModel));
		LogDeviceData("Shader Model Support " + std::string(EShaderSupportModel::ToString(Caps_Data.HighestModel)));
	}

#if WIN10_1903
	ZeroMemory(&DeviceFeatureData.FeatureData6, sizeof(DeviceFeatureData.FeatureData6));
	hr = m_Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS6, &DeviceFeatureData.FeatureData6, sizeof(DeviceFeatureData.FeatureData3));
	if (SUCCEEDED(hr))
	{
		if (LogDeviceDebug)
		{
			LogTierData("VRS Support", DeviceFeatureData.FeatureData6.VariableShadingRateTier, "(Tile size " + std::to_string(DeviceFeatureData.FeatureData6.ShadingRateImageTileSize) + ")");
		}
		Caps_Data.VRSSupport = DeviceFeatureData.FeatureData6.VariableShadingRateTier != D3D12_VARIABLE_SHADING_RATE_TIER_NOT_SUPPORTED ? EVRSSupportType::Hardware : EVRSSupportType::None;
		if (DeviceFeatureData.FeatureData6.VariableShadingRateTier >= D3D12_VARIABLE_SHADING_RATE_TIER_1)
		{
			if (DeviceFeatureData.FeatureData6.VariableShadingRateTier >= D3D12_VARIABLE_SHADING_RATE_TIER_2)
			{
				Caps_Data.VRSSupport = EVRSSupportType::Hardware_Tier2;
			}
			Caps_Data.VRSTileSize = DeviceFeatureData.FeatureData6.ShadingRateImageTileSize;
		}
	}
	else
#endif
	{
		Caps_Data.VRSSupport = EVRSSupportType::None;
		Caps_Data.VRSTileSize = RHI::GetRenderSettings()->GetVRXSettings().VRSTileSize;
	}
	if (options.CrossNodeSharingTier != D3D12_CROSS_NODE_SHARING_TIER_NOT_SUPPORTED)
	{
		switch (options.CrossNodeSharingTier)
		{
		case D3D12_CROSS_NODE_SHARING_TIER_1_EMULATED:
			Caps_Data.ConnectionMode = EMGPUConnectionMode::HostStagedTransfer;
			break;
		case D3D12_CROSS_NODE_SHARING_TIER_1:
			Caps_Data.ConnectionMode = EMGPUConnectionMode::DirectTransfer;
			break;
		case D3D12_CROSS_NODE_SHARING_TIER_2:
			Caps_Data.ConnectionMode = EMGPUConnectionMode::DirectTransfer;
			break;
		}
		//#DXMGPU: Detect what the other GPU is.
	}
	else
	{
		Caps_Data.ConnectionMode = EMGPUConnectionMode::None;
	}
	Caps_Data.SupportsDepthBoundsTest = DeviceFeatureData.FeatureData2.DepthBoundsTestSupported;
	Caps_Data.SupportExecuteIndirect = true;//all D3D12 GPUs support draw indirect etc.
	LogDeviceData("InterGPU mode " + std::string(EMGPUConnectionMode::ToString(Caps_Data.ConnectionMode)));
	CheckNVAPISupport();
#if 0
	ID3D12VideoDevice * VideoDevice;
	m_Device->QueryInterface(ID_PASS(&VideoDevice));
	D3D12_FEATURE_DATA_VIDEO_DECODE_PROFILE_COUNT  FMT;
	ZeroMemory(&FMT, sizeof(FMT));
	FMT.NodeIndex = 0;
	HRESULT r = VideoDevice->CheckFeatureSupport(D3D12_FEATURE_VIDEO::D3D12_FEATURE_VIDEO_DECODE_PROFILE_COUNT, &FMT, sizeof(FMT));

	D3D12_FEATURE_DATA_VIDEO_DECODE_SUPPORT Data;
	Data.NodeIndex = 0;
	Data.Configuration.DecodeProfile = D3D12_VIDEO_DECODE_PROFILE_HEVC_MAIN;
	Data.Configuration.BitstreamEncryption = D3D12_BITSTREAM_ENCRYPTION_TYPE_NONE;
	Data.Configuration.InterlaceType = D3D12_VIDEO_FRAME_CODED_INTERLACE_TYPE_NONE;
	VideoDevice->CheckFeatureSupport(D3D12_FEATURE_VIDEO::D3D12_FEATURE_VIDEO_DECODE_SUPPORT, &Data, sizeof(Data));
	//VideoDevice->CreateVideoProcessor();
#endif

	if (DeviceFeatureData.FeatureData.TiledResourcesTier >= D3D12_TILED_RESOURCES_TIER_2)
	{
		Caps_Data.SamplerFeedbackMode = ESamplerFeedBackSupportMode::Emulated;
	}
	else
	{
		Caps_Data.SamplerFeedbackMode = ESamplerFeedBackSupportMode::None;
	}
#if WIN10_2004	
	if (SUCCEEDED(m_Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &DeviceFeatureData.FeatureData7, sizeof(DeviceFeatureData))))
	{
		if (DeviceFeatureData.FeatureData7.SamplerFeedbackTier > D3D12_SAMPLER_FEEDBACK_TIER_NOT_SUPPORTED)
		{
			Caps_Data.SamplerFeedbackMode = ESamplerFeedBackSupportMode::FullHardware;
		}
		if (DeviceFeatureData.FeatureData7.MeshShaderTier > D3D12_MESH_SHADER_TIER_NOT_SUPPORTED)
		{
			Caps_Data.SupportsMeshShaders = true;
		}
	}
#endif
}

void D3D12DeviceContext::LogDeviceData(const std::string& data)
{
	Log::LogMessage("Device " + std::to_string(GetDeviceIndex()) + ": " + data);
}

void D3D12DeviceContext::LogTierData(const std::string& data, int teir, const std::string& extramsg)
{
	LogDeviceData(data + " tier " + std::to_string(teir) + " " + extramsg);
}

void D3D12DeviceContext::InitDevice(int index)
{
	DeviceIndex = index;
	//#SLI Mask needs to be set correctly to handle mixed SLI 
	CheckFeatures();
#if 0
	pDXGIAdapter->RegisterVideoMemoryBudgetChangeNotificationEvent(m_VideoMemoryBudgetChange, &m_BudgetNotificationCookie);
#endif
	m_Device->QueryInterface(ID_PASS(&m_Device2));
#if WIN10_1809
	m_Device->QueryInterface(ID_PASS(&m_Device5));
#endif
#if WIN10_1903
	m_Device->QueryInterface(ID_PASS(&m_device6));
#endif
	HeapManager = new DescriptorHeapManager(this);
	DescriptorCacheManager = new DescriptorCache(this);
	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.NodeMask = GetNodeMask();

	ThrowIfFailed(m_Device->CreateCommandQueue(&queueDesc, ID_PASS(&m_MainCommandQueue)));
	DEVICE_NAME_OBJECT(m_MainCommandQueue);
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
	ThrowIfFailed(m_Device->CreateCommandQueue(&queueDesc, ID_PASS(&m_ComputeCommandQueue)));

	queueDesc.Type = D3D12RHIConfig::CopyListQueueType;
	ThrowIfFailed(m_Device->CreateCommandQueue(&queueDesc, ID_PASS(&m_CopyCommandQueue)));
#if SUPPORT_DXGI
	queueDesc.Type = D3D12RHIConfig::CopyListQueueType;
	ThrowIfFailed(m_Device->CreateCommandQueue(&queueDesc, ID_PASS(&m_SharedCopyCommandQueue)));
	DEVICE_NAME_OBJECT(m_SharedCopyCommandQueue);
#endif

	DEVICE_NAME_OBJECT(m_ComputeCommandQueue);
	DEVICE_NAME_OBJECT(m_CopyCommandQueue);

	GraphicsQueueSync.Init(GetDevice());
	CopyQueueSync.Init(GetDevice());
	ComputeQueueSync.Init(GetDevice());
	GpuWaitSyncPoint.InitGPUOnly(GetDevice());

	TimeManager = new D3D12TimeManager(this);
	GPUCopyList = new D3D12CommandList(this, ECommandListType::Copy);
	InterGPUCopyList = new D3D12CommandList(this, ECommandListType::Copy);
	D3D12RHI::DXConv(GPUCopyList)->CreateCommandList();
	GPUCopyList->ResetList();
	GraphicsSync.Init(GetCommandQueueFromEnum(DeviceContextQueue::Graphics), GetDevice());
	CopySync.Init(GetCommandQueueFromEnum(DeviceContextQueue::Copy), GetDevice());
#if SUPPORT_DXGI
	InterGPUSync.Init(GetCommandQueueFromEnum(DeviceContextQueue::InterCopy), GetDevice());
#endif
	ComputeSync.Init(GetCommandQueueFromEnum(DeviceContextQueue::Compute), GetDevice());
	for (int x = 0; x < RHI::CPUFrameCount; x++)
	{
		for (int i = 0; i < DeviceContextQueue::LIMIT; i++)
		{
			GPUWaitPoints[x][i].InitGPUOnly(GetDevice());
		}
	}
	if (EnableStablePower.GetBoolValue())
	{
		Log::LogMessage("SetStablePowerState is enabled GPU clocks are locked at Base frequencies", Log::Error);
		GetDevice()->SetStablePowerState(true);
	}
	TimeStampHeap = new D3D12QueryHeap(this, 8192, D3D12_QUERY_HEAP_TYPE_TIMESTAMP);
	CopyTimeStampHeap = new D3D12QueryHeap(this, 512, D3D12_QUERY_HEAP_TYPE_COPY_QUEUE_TIMESTAMP);
	PipelinePerfHeap = new D3D12QueryHeap(this, 512, D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS);
	SampleVideoMemoryInfo();
	MemoryManager = new DXMemoryManager(this);
	InitCopyListPool();
	Streamer = new DXGPUTextureStreamer();
	Streamer->Init(this);
	PostInit();
}

void D3D12DeviceContext::CreateNodeDevice(ID3D12Device* dev, int nodemask, int index)
{
	m_Device = dev;
	Log::LogMessage("Creating Node device " + std::to_string(nodemask) + "/" + std::to_string(m_Device->GetNodeCount()) + " Nodes");
	SetNodeMaskFromIndex(nodemask);
	InitDevice(index);
}

DXMemoryManager* D3D12DeviceContext::GetMemoryManager()
{
	return MemoryManager;
}

DeviceMemoryData D3D12DeviceContext::GetMemoryData()
{
	return MemoryData;
}

D3D_SHADER_MODEL D3D12DeviceContext::GetShaderModel() const
{
	return HighestShaderModel;
}

void D3D12DeviceContext::EnqueueUploadRequest(const GPUUploadRequest& request)
{
	Requests.push_back(request);
}

CommandAllocator* D3D12DeviceContext::GetAllocator(D3D12CommandList* list)
{
	//todo: match threads
	for (int i = 0; i < Allocators.size(); i++)
	{
		if (!Allocators[i]->IsInUse() && Allocators[i]->GetType() == list->GetListType())
		{
			return Allocators[i];
		}
	}
	CommandAllocator* Alloc = new CommandAllocator(list->GetListType(), this);
	Allocators.push_back(Alloc);
	return Alloc;
}

const DXFeatureData& D3D12DeviceContext::GetFeatureData() const
{
	return DeviceFeatureData;
}

bool D3D12DeviceContext::IsUMA() const
{
	return DeviceFeatureData.ArchData.UMA;
}
#ifdef PLATFORM_WINDOWS
std::string GetDXRstring(D3D12_RAYTRACING_TIER t)
{
	if (t == D3D12_RAYTRACING_TIER_1_0)
	{
		return "DXR_TIER_1_0";
	}
	return "DXR_TIER_NOT_SUPPORTED";
}
std::string GetNiceTierData(int i)
{
	if (i == 0)
	{
		return "NOT_SUPPORTED";
	}
	return "TIER_" + std::to_string(i);
}
#endif
std::string D3D12DeviceContext::ReportDeviceData()
{
	std::string Data = "GPU " + std::to_string(GetDeviceIndex()) + ":\n";
	int Length = 30;
	Data += StringUtils::PadToLength("Adapter Name", Length) + StringUtils::ConvertWideToString(Adaptordesc.Description) + "\n";
	Data += StringUtils::PadToLength("Type", Length) + EGPUType::ToString(GetType()) + "\n";
	Data += StringUtils::PadToLength("Dedicated Video Memory", Length) + StringUtils::ByteToGB(Adaptordesc.DedicatedVideoMemory) + "\n";
#ifdef PLATFORM_WINDOWS 
	Data += StringUtils::PadToLength("DXR Tier", Length) + GetDXRstring(DeviceFeatureData.FeatureData5.RaytracingTier) + "\n";
	Data += StringUtils::PadToLength("VRS Tier", Length) + GetNiceTierData(DeviceFeatureData.FeatureData6.VariableShadingRateTier) + "\n";
	Data += StringUtils::PadToLength("Typed UAV Load Support", Length) + StringUtils::BoolToString(DeviceFeatureData.FeatureData.TypedUAVLoadAdditionalFormats) + "\n";
#endif
	return Data;
}

void D3D12DeviceContext::FlushUploadQueue()
{
	for (int i = 0; i < Requests.size(); i++)
	{
		GPUUploadRequest* R = &Requests[i];
		UpdateSubresources(GetCopyList(), R->Target->GetResource(), R->UploadBuffer->GetResource(), 0, 0, R->SubResourceDesc.size(), &R->SubResourceDesc[0]);
		RHI::AddToDeferredDeleteQueue(R->UploadBuffer);
		delete R->DataPtr;
	}
	Requests.clear();
}

void D3D12DeviceContext::CreateDeviceFromAdaptor(IDXGIAdapter1* adapter, int index)
{
#if SUPPORT_DXGI
	EnableStablePower.SetValue(true);
	pDXGIAdapter = (IDXGIAdapter3*)adapter;
	pDXGIAdapter->GetDesc1(&Adaptordesc);
	VendorID = Adaptordesc.VendorId;

	HRESULT result = D3D12CreateDevice(
		pDXGIAdapter,
		D3D_FEATURE_LEVEL_11_0,
		ID_PASS(&m_Device)
	);
	ensureFatalMsgf(!(result == DXGI_ERROR_UNSUPPORTED), "D3D_FEATURE_LEVEL_11_0 is required to run this engine");
	ThrowIfFailed(result);
	D3D_FEATURE_LEVEL MaxLevel = D3D12RHI::GetMaxSupportedFeatureLevel(m_Device);
	if (MaxLevel != D3D_FEATURE_LEVEL_11_0)
	{
		m_Device->Release();
		ThrowIfFailed(D3D12CreateDevice(
			pDXGIAdapter,
			MaxLevel,
			ID_PASS(&m_Device)
		));
	}
	DEVICE_NAME_OBJECT(m_Device);


	if (LogDeviceDebug)
	{
		std::stringstream ss;
		ss << "Device " << std::to_string(GetDeviceIndex()) << " Created With Feature level " << D3D12Helpers::StringFromFeatureLevel(MaxLevel);
		Log::LogMessage(ss.str());
		Log::LogMessage("Creating device with " + std::to_string(m_Device->GetNodeCount()) + " Nodes");
	}
	SetNodeMaskFromIndex(0);
	InitDevice(index);
#endif

}

void D3D12DeviceContext::LinkAdaptors(D3D12DeviceContext* other)
{
	CrossAdaptorSync[0].Init(GetDevice(), other->GetDevice());
	CrossAdaptorSync[1].Init(GetDevice(), other->GetDevice());
}

ID3D12Device* D3D12DeviceContext::GetDevice()
{
	return m_Device;
}

ID3D12Device2* D3D12DeviceContext::GetDevice2()
{
	return m_Device2;
}

#if WIN10_1809
ID3D12Device5* D3D12DeviceContext::GetDevice5()
{
	return m_Device5;
}
#endif
#if WIN10_1903
ID3D12Device6* D3D12DeviceContext::GetDevice6()
{
	return m_device6;
}
#endif
ID3D12CommandQueue* D3D12DeviceContext::GetCommandQueue()
{
	return GetCommandQueueFromEnum(DeviceContextQueue::Graphics);
}

void D3D12DeviceContext::MoveNextFrame(int SyncIndex)
{
	//force Queue sync at the end of frame
	if (!AllowCrossFrameAsyncCompute)
	{
		InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::Compute);
		InsertGPUWait(DeviceContextQueue::Compute, DeviceContextQueue::Graphics);
	}
#if 1
	if (GetDeviceIndex() != 0)
	{
		InsertCrossGPUWait(DeviceContextQueue::Graphics, RHI::GetDeviceContext(0), DeviceContextQueue::Graphics);
	}
#endif
	GraphicsSync.MoveNextFrame(SyncIndex);
	CopySync.MoveNextFrame(SyncIndex);
	InterGPUSync.MoveNextFrame(SyncIndex);
	ComputeSync.MoveNextFrame(SyncIndex);
	CurrentFrameIndex = SyncIndex;
	GetHeapManager()->ClearMainHeap();
	DescriptorCacheManager->OnHeapClear();
	//DescriptorCacheManager->Invalidate();
	IncrementDeviceFrame();
}

void D3D12DeviceContext::ResetDeviceAtEndOfFrame()
{
	DeviceContext::ResetDeviceAtEndOfFrame();
	HeapManager->EndOfFrame();
	ResetCopyEngine();
	if (RHI::GetFrameCount() == 1 || RHI::GetFrameCount() == 100)
	{
		GetMemoryManager()->LogMemoryReport();
	}
	//#dxmm: Trigger this post load
	if (RHI::GetFrameCount() == 10)
	{
		GetMemoryManager()->Compact();
	}
}

void D3D12DeviceContext::SampleVideoMemoryInfo()
{
#if SUPPORT_DXGI
	pDXGIAdapter->QueryVideoMemoryInfo(GetNodeIndex(), DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL, &CurrentVideoMemoryInfo);
	MemoryData.HostSegment_TotalBytes = CurrentVideoMemoryInfo.Budget;

	pDXGIAdapter->QueryVideoMemoryInfo(GetNodeIndex(), DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &CurrentVideoMemoryInfo);

	usedVRAM = CurrentVideoMemoryInfo.CurrentUsage / 1024 / 1024;
	totalVRAM = CurrentVideoMemoryInfo.Budget / 1024 / 1024;
	MemoryData.LocalSegment_TotalBytes = CurrentVideoMemoryInfo.Budget;
	MemoryData.Local_Usage = CurrentVideoMemoryInfo.CurrentUsage;
	if (GetMemoryManager())
	{
		GetMemoryManager()->UpdateTotalAlloc();
	}
#endif
}

RHIClass::GPUMemoryData D3D12DeviceContext::GetMemoryReport()
{
	RHIClass::GPUMemoryData data = {};
	data.MaxPhysical = Adaptordesc.DedicatedVideoMemory;
	data.TotalAllocated = GetMemoryManager()->GetTotalAllocated();
	data.UntrackedDelta = (int64_t)MemoryData.Local_Usage - (int64_t)GetMemoryManager()->GetTotalReserved();
	data.MaxBudget = MemoryData.LocalSegment_TotalBytes;
	return data;
}

void D3D12DeviceContext::DestoryDevice()
{
	DeviceContext::DestoryDevice();
	CopyEngineHasWork = true;
	UpdateCopyEngine();
	CPUWaitForAll();
	RHI::FlushDeferredDeleteQueue();
}

void D3D12DeviceContext::WaitForGpu()
{
	GraphicsQueueSync.CreateSyncPoint(m_MainCommandQueue);
}

void D3D12DeviceContext::WaitForCopy()
{
#if  1//SUPPORT_DXGI	
	CopyQueueSync.CreateSyncPoint(m_CopyCommandQueue);
#endif
}

void D3D12DeviceContext::ReportData()
{
	std::string VendorString = "";
	if (IsDeviceNVIDIA())
	{
		VendorString = "NVIDIA";
	}
	else if (IsDeviceAMD())
	{
		VendorString = "AMD";
	}
	else if (IsDeviceIntel())
	{
		VendorString = "Intel";
	}

	//Memory and name
	std::stringstream ss;
	ss << VendorString << " Adapter Name: \"" << StringUtils::ConvertWideToString(Adaptordesc.Description) << "\" (" << EGPUType::ToString(GetType()) << ") " << " Dedicated Video Memory: " << std::setprecision(3) << Adaptordesc.DedicatedVideoMemory / 1e9 << "GB ";
	Log::LogMessage(ss.str());
}

CopyCMDListType* D3D12DeviceContext::GetCopyList()
{
	return D3D12RHI::DXConv(GPUCopyList)->GetCopyList();
}
D3D12CommandList* D3D12DeviceContext::GetCopyListDx()
{
	return D3D12RHI::DXConv(GPUCopyList);
}
void D3D12DeviceContext::NotifyWorkForCopyEngine()
{
	CopyEngineHasWork = true;
}

void D3D12DeviceContext::UpdateCopyEngine()
{
	if (Requests.size() > 0 || CopyEngineHasWork)
	{
		FlushUploadQueue();
		GPUCopyList->Execute();
		WaitForCopy();
		CopyEngineHasWork = false;
	}

}

void D3D12DeviceContext::ResetCopyEngine()
{
	if (!GPUCopyList->IsOpen())
	{
		GPUCopyList->ResetList();
	}
}

void D3D12DeviceContext::ExecuteComputeCommandList(ID3D12GraphicsCommandList* list)
{
	ID3D12CommandList* ppCommandLists[] = { list };
	m_ComputeCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	if (RHI::BlockCommandlistExec())
	{
		ComputeQueueSync.CreateSyncPoint(m_ComputeCommandQueue);
	}
}

void D3D12DeviceContext::ExecuteCopyCommandList(CopyCMDListType* list)
{
	ID3D12CommandList* ppCommandLists[] = { list };
	m_CopyCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	if (RHI::BlockCommandlistExec())
	{
		CopyQueueSync.CreateSyncPoint(m_CopyCommandQueue);
	}
}

void D3D12DeviceContext::ExecuteInterGPUCopyCommandList(ID3D12GraphicsCommandList* list, bool forceblock)
{
#if SUPPORT_DXGI	
	ID3D12CommandList* ppCommandLists[] = { list };
	m_SharedCopyCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	if (RHI::BlockCommandlistExec() || forceblock)
	{
		GraphicsQueueSync.CreateSyncPoint(m_SharedCopyCommandQueue);
	}
#endif
}
void D3D12DeviceContext::ExecuteCommandList(ID3D12GraphicsCommandList* list)
{
	ID3D12CommandList* ppCommandLists[] = { list };
	GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	if (RHI::BlockCommandlistExec())
	{
		WaitForGpu();
	}
}

RHITimeManager* D3D12DeviceContext::GetTimeManager()
{
	return TimeManager;
}

void D3D12DeviceContext::GPUWaitForOtherGPU(DeviceContext* OtherGPU, DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue)
{
	CrossAdaptorSync[GetCpuFrameIndex()].CrossGPUCreateSyncPoint(GetCommandQueueFromEnum(SignalQueue), D3D12RHI::DXConv(OtherGPU)->GetCommandQueueFromEnum(WaitingQueue));
}

void D3D12DeviceContext::InsertCrossGPUWait(DeviceContextQueue::Type WaitingQueue, DeviceContext* SignalingGPU, DeviceContextQueue::Type SignalQueue)
{
	CrossAdaptorSync[GetCpuFrameIndex()].CrossGPUCreateSyncPoint_NonLocalSignal(D3D12RHI::DXConv(SignalingGPU)->GetCommandQueueFromEnum(SignalQueue), GetCommandQueueFromEnum(WaitingQueue));
}

bool D3D12DeviceContext::SupportsCommandList4()
{
	return SupportsCmdsList4;
}

void D3D12DeviceContext::CPUWaitForAll()
{
	GraphicsQueueSync.CreateSyncPoint(m_MainCommandQueue);
#if SUPPORT_DXGI	
	CopyQueueSync.CreateSyncPoint(m_SharedCopyCommandQueue);
#endif
	CopyQueueSync.CreateSyncPoint(m_CopyCommandQueue);
	ComputeQueueSync.CreateSyncPoint(m_ComputeCommandQueue);
}

ID3D12CommandQueue* D3D12DeviceContext::GetCommandQueueFromEnum(DeviceContextQueue::Type value)
{
	switch (value)
	{
	case DeviceContextQueue::Graphics:
		return m_MainCommandQueue;
		break;
	case DeviceContextQueue::Compute:
		return m_ComputeCommandQueue;
		break;
	case DeviceContextQueue::Copy:
		return m_CopyCommandQueue;
		break;
	case DeviceContextQueue::InterCopy:
		return m_SharedCopyCommandQueue;
		break;
	}
	return nullptr;
}

void D3D12DeviceContext::InsertGPUWait(DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue)
{
	SCOPE_CYCLE_COUNTER_GROUP("InsertGPUWait", "RHI");
#if 0
	GPUWaitPoints[0][SignalQueue].GPUCreateSyncPoint(GetCommandQueueFromEnum(SignalQueue), GetCommandQueueFromEnum(WaitingQueue));
#else
	SignalCommandQueue(SignalQueue);
	InsertWaitForValue(SignalQueue, WaitingQueue);
#endif
}


RHICommandList* D3D12DeviceContext::GetInterGPUCopyList()
{
	return InterGPUCopyList;
}

DescriptorHeapManager* D3D12DeviceContext::GetHeapManager()
{
	return HeapManager;
}

DescriptorCache* D3D12DeviceContext::GetDescriptorCache()
{
	return DescriptorCacheManager;
}

D3D12QueryHeap* D3D12DeviceContext::GetTimeStampHeap()
{
	return TimeStampHeap;
}

D3D12QueryHeap* D3D12DeviceContext::GetCopyTimeStampHeap()
{
	return CopyTimeStampHeap;
}

void D3D12DeviceContext::OnFrameStart()
{
	GetTimeManager()->UpdateTimers();
	TimeStampHeap->BeginQuerryBatch();
	if (CopyTimeStampHeap)
	{
		CopyTimeStampHeap->BeginQuerryBatch();
	}
}

bool D3D12DeviceContext::IsPartOfNodeGroup()
{
	return m_Device->GetNodeCount() > 1;
}

int D3D12DeviceContext::GetNodeCount()
{
	return m_Device->GetNodeCount();
}

GPUSyncPoint::~GPUSyncPoint()
{
	SafeRelease(m_fence);
	SafeRelease(secondaryFence);
}

void GPUSyncPoint::Init(ID3D12Device* device, ID3D12Device* SecondDevice)
{
	//Fence types
	//  D3D12_FENCE_FLAG_NONE
	//  D3D12_FENCE_FLAG_SHARED
	//	D3D12_FENCE_FLAG_SHARED_CROSS_ADAPTER
	//	D3D12_FENCE_FLAG_NON_MONITORED
	ThrowIfFailed(device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_SHARED | D3D12_FENCE_FLAG_SHARED_CROSS_ADAPTER, ID_PASS(&m_fence)));
	m_fenceEvent = CreateEvent(nullptr, false, false, nullptr);
	if (m_fenceEvent == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}
	m_fenceValue++;
	// Create a shared handle to the cross adapter fence
	HANDLE fenceHandle = nullptr;
	device->CreateSharedHandle(
		m_fence,
		nullptr,
		GENERIC_ALL,
		nullptr,
		&fenceHandle);

	// Open shared handle to fence on secondaryDevice GPU
	SecondDevice->OpenSharedHandle(fenceHandle, ID_PASS(&secondaryFence));

	CloseHandle(fenceHandle);
}
void GPUSyncPoint::InitGPUOnly(ID3D12Device* device)
{
	ThrowIfFailed(device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, ID_PASS(&m_fence)));
	m_fenceValue++;
}
void GPUSyncPoint::Init(ID3D12Device* device)
{
	//Fence types
	//  D3D12_FENCE_FLAG_NONE
	//  D3D12_FENCE_FLAG_SHARED
	//	D3D12_FENCE_FLAG_SHARED_CROSS_ADAPTER
	//	D3D12_FENCE_FLAG_NON_MONITORED
	ThrowIfFailed(device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, ID_PASS(&m_fence)));
	m_fenceEvent = CreateEvent(nullptr, false, false, nullptr);
	if (m_fenceEvent == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}
	m_fenceValue++;
}

void GPUSyncPoint::CreateSyncPoint(ID3D12CommandQueue* queue)
{
	// Schedule a Signal command in the queue.
	ThrowIfFailed(queue->Signal(m_fence, m_fenceValue));

	// Wait until the fence has been processed.
	ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent));
	DWORD status = WaitForSingleObject(m_fenceEvent, INFINITE);
	if (status == WAIT_OBJECT_0)
	{
		// Increment the fence value for the current frame.
		m_fenceValue++;
		return;
	}
	ensure(status == WAIT_OBJECT_0);
}
void GPUSyncPoint::CrossGPUCreateSyncPoint(ID3D12CommandQueue* queue, ID3D12CommandQueue* otherDeviceQeue)
{
	// Schedule a Signal command in the queue.
	ThrowIfFailed(queue->Signal(m_fence, m_fenceValue));

	otherDeviceQeue->Wait(secondaryFence, m_fenceValue);
	m_fenceValue++;
}

void GPUSyncPoint::CrossGPUCreateSyncPoint_NonLocalSignal(ID3D12CommandQueue* queue, ID3D12CommandQueue* otherDeviceQeue)
{
	// Schedule a Signal command in the queue.
	ThrowIfFailed(queue->Signal(secondaryFence, m_fenceValue));

	otherDeviceQeue->Wait(m_fence, m_fenceValue);
	m_fenceValue++;
}

void GPUSyncPoint::GPUCreateSyncPoint(ID3D12CommandQueue* queue, ID3D12CommandQueue* targetqueue)
{
	//Breaks!
	// Schedule a Signal command in the queue.
	ThrowIfFailed(queue->Signal(m_fence, m_fenceValue));

	targetqueue->Wait(m_fence, m_fenceValue);
	m_fenceValue++;
}

void GPUSyncPoint::Signal(ID3D12CommandQueue* queue, int value)
{
	if (value != -1)
	{
		m_fenceValue = value;
	}
	ThrowIfFailed(queue->Signal(m_fence, m_fenceValue));
}

void GPUSyncPoint::Wait(ID3D12CommandQueue* queue, int value)
{
	if (value != -1)
	{
		m_fenceValue = value;
	}
	ThrowIfFailed(queue->Wait(m_fence, m_fenceValue));
	m_fenceValue++;
}

void GPUFenceSync::Init(ID3D12CommandQueue* TargetQueue, ID3D12Device* device)
{
	Queue = TargetQueue;
	ThrowIfFailed(device->CreateFence(m_fenceValues[m_frameIndex], D3D12_FENCE_FLAG_NONE, ID_PASS(&m_fence)));
	m_fenceValues[m_frameIndex] = 1;
	// Create an event handle to use for frame synchronization
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
#if !BUILD_SHIPPING
	if (m_fenceEvent == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}
#endif
#if NAME_RHI_PRIMS
	m_fence->SetName(L"GPUFenceSync Fence");
#endif
}

void GPUFenceSync::MoveNextFrame(int SyncIndex)
{
	if (Queue == nullptr)
	{
		return;
	}
	// Schedule a Signal command in the queue.
	const UINT64 currentFenceValue = m_fenceValues[m_frameIndex];
	ThrowIfFailed(Queue->Signal(m_fence, currentFenceValue));
	// Update the frame index.
	m_frameIndex = SyncIndex;

	// If the next frame is not ready to be rendered yet, wait until it is ready.
	const UINT64 value = m_fence->GetCompletedValue();
	if (value < m_fenceValues[m_frameIndex])
	{
		ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
		WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
	}

	// Set the fence value for the next frame.
	m_fenceValues[m_frameIndex] = currentFenceValue + 1;
}

D3D12GPUSyncEvent::D3D12GPUSyncEvent(DeviceContextQueue::Type WaitingQueueEnum, DeviceContextQueue::Type SignalQueueEnum, DeviceContext* device, DeviceContext* OtherDevice) :RHIGPUSyncEvent(WaitingQueueEnum, SignalQueueEnum, device)
{

	D3D12DeviceContext* d3dc = D3D12RHI::DXConv(Device);
	if (OtherDevice == nullptr)
	{
		for (int i = 0; i < RHI::CPUFrameCount; i++)
		{
			Point[i].InitGPUOnly(d3dc->GetDevice());
		}
		WaitingQueue = d3dc->GetCommandQueueFromEnum(WaitingQueueEnum);
		SignalQueue = d3dc->GetCommandQueueFromEnum(SignalQueueEnum);
	}
	else
	{
		SignalingDevice = OtherDevice;
		D3D12DeviceContext* sDevice = D3D12RHI::DXConv(SignalingDevice);
		for (int i = 0; i < RHI::CPUFrameCount; i++)
		{
			Point[i].Init(sDevice->GetDevice(), d3dc->GetDevice());
		}
		WaitingQueue = d3dc->GetCommandQueueFromEnum(WaitingQueueEnum);
		SignalQueue = sDevice->GetCommandQueueFromEnum(SignalQueueEnum);
	}
}

D3D12GPUSyncEvent::~D3D12GPUSyncEvent()
{}

void D3D12GPUSyncEvent::SignalWait()
{
	Point[Device->GetCpuFrameIndex()].CrossGPUCreateSyncPoint(SignalQueue, WaitingQueue);
}
void D3D12GPUSyncEvent::Signal()
{
	Point[Device->GetCpuFrameIndex()].Signal(SignalQueue);
}

void D3D12GPUSyncEvent::Wait()
{
	Point[Device->GetCpuFrameIndex()].Wait(WaitingQueue);
}

void D3D12DeviceContext::CheckNVAPISupport()
{
#if NVAPI_PRESENT
	NV_QUERY_SINGLE_PASS_STEREO_SUPPORT_PARAMS Par = {};
	Par.version = NV_QUERY_SINGLE_PASS_STEREO_SUPPORT_PARAMS_VER1;
	NvAPI_Status ret = NVAPI_OK;
	ret = NvAPI_D3D12_QuerySinglePassStereoSupport(GetDevice(), &Par);
	if (ret == NvAPI_Status::NVAPI_OK && Par.bSinglePassStereoSupported)
	{
		LogDeviceData("Supports single pass stereo");
	}
#endif
}