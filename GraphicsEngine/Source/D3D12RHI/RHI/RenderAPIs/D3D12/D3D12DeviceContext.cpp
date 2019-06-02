
#include "D3D12DeviceContext.h"
#include "D3D12TimeManager.h"
#include "D3D12CommandList.h"
#include "D3D12RHI.h"
#include "Core/Performance/PerfManager.h"
#include "DescriptorHeapManager.h"
#include "D3D12QueryHeap.h"
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
	SafeRelease(m_MainCommandQueue);
	for (int i = 0; i < RHI::CPUFrameCount; i++)
	{
		SafeRelease(m_commandAllocator[i]);
		SafeRelease(m_SharedCopyCommandAllocator[i]);
	}
	SafeRelease(m_CopyCommandAllocator);
	SafeDelete(TimeManager);
	SafeRHIRelease(GPUCopyList);
	SafeRHIRelease(InterGPUCopyList);
	SafeDelete(TimeStampHeap);
	SafeDelete(CopyTimeStampHeap);
	SafeRelease(m_IntraCopyList);
	SafeRelease(m_SharedCopyCommandQueue);
	SafeRelease(m_ComputeCommandQueue);
	SafeRelease(m_CopyCommandQueue);
	SafeRelease(m_Device);
	SafeDelete(HeapManager);
	SafeRelease(pDXGIAdapter);
	/*if (pDXGIAdapter != nullptr)
	{
		pDXGIAdapter->UnregisterVideoMemoryBudgetChangeNotification(m_BudgetNotificationCookie);
	}*/

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
#if 0
	Log::LogMessage("Device " + std::to_string(GetDeviceIndex()) + " CrossAdapterRowMajorTextureSupported " + (options.CrossAdapterRowMajorTextureSupported ? "true " : "false "));
	Log::LogMessage("Device " + std::to_string(GetDeviceIndex()) + " CrossNodeSharingTier " + std::to_string(options.CrossNodeSharingTier));
#endif
	D3D12_FEATURE_DATA_ARCHITECTURE1 ARCHDAta = {};
	ARCHDAta.NodeIndex = 0;
	ThrowIfFailed(GetDevice()->CheckFeatureSupport(D3D12_FEATURE_ARCHITECTURE1, reinterpret_cast<void*>(&ARCHDAta), sizeof(ARCHDAta)));
	if (ARCHDAta.UMA)
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
#if 0
	LogFeatureData("UMA", ARCHDAta.UMA);
	LogFeatureData("TileBasedRenderer", ARCHDAta.TileBasedRenderer);
	LogFeatureData("IsolatedMMU", ARCHDAta.IsolatedMMU);
#endif

	ReportData();
	D3D12_FEATURE_DATA_D3D12_OPTIONS  FeatureData;
	ZeroMemory(&FeatureData, sizeof(FeatureData));
	HRESULT hr = m_Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &FeatureData, sizeof(FeatureData));
	if (SUCCEEDED(hr) && LogDeviceDebug)
	{
		LogTierData("Resource Binding", FeatureData.ResourceBindingTier);
		LogTierData("Resource Heap", FeatureData.ResourceHeapTier);
		LogTierData("Cross Node Sharing", FeatureData.CrossNodeSharingTier);
	}
	D3D12_FEATURE_DATA_D3D12_OPTIONS1  FeatureData1;
	ZeroMemory(&FeatureData1, sizeof(FeatureData1));
	hr = m_Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS1, &FeatureData1, sizeof(FeatureData1));
	if (SUCCEEDED(hr) && LogDeviceDebug)
	{
		LogDeviceData("Threads Per warp count: " + std::to_string(FeatureData1.WaveLaneCountMin));
	}
	D3D12_FEATURE_DATA_D3D12_OPTIONS5  FeatureData5;
	ZeroMemory(&FeatureData5, sizeof(FeatureData5));
	hr = m_Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &FeatureData5, sizeof(FeatureData5));
	if (SUCCEEDED(hr) && LogDeviceDebug)
	{
		LogTierData("DXR Tier", FeatureData5.RaytracingTier);
		//#DXR Detect driver RT
		Caps_Data.RTSupport = ERayTracingSupportType::Hardware;
		SupportsCmdsList4 = true;
	}
	else
	{
		Log::LogMessage("System does not support DXR");
		Caps_Data.RTSupport = ERayTracingSupportType::Software;
		SupportsCmdsList4 = false;
	}

	D3D12_FEATURE_DATA_D3D12_OPTIONS3  FeatureData3;
	ZeroMemory(&FeatureData3, sizeof(FeatureData3));
	hr = m_Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS3, &FeatureData3, sizeof(FeatureData3));
	if (SUCCEEDED(hr) && LogDeviceDebug)
	{
		LogTierData("VIEW INSTANCING", FeatureData3.ViewInstancingTier);
		Caps_Data.SupportsCopyTimeStamps = FeatureData3.CopyQueueTimestampQueriesSupported;
		Caps_Data.SupportsViewInstancing = (FeatureData3.ViewInstancingTier > D3D12_VIEW_INSTANCING_TIER_NOT_SUPPORTED);
	}
	//todo loop
	D3D12_FEATURE_DATA_SHADER_MODEL  ShaderModelData = {};
	ShaderModelData.HighestShaderModel = D3D_SHADER_MODEL_6_0;
	hr = m_Device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &ShaderModelData, sizeof(ShaderModelData));
	if (SUCCEEDED(hr) && LogDeviceDebug)
	{
		LogDeviceData("SM " + std::to_string(ShaderModelData.HighestShaderModel));
	}
}

void D3D12DeviceContext::LogDeviceData(const std::string& data)
{
	Log::LogMessage("Device " + std::to_string(GetDeviceIndex()) + " " + data);
}

void D3D12DeviceContext::LogTierData(const std::string& data, int teir)
{
	LogDeviceData(data + " tier " + std::to_string(teir));
}

void D3D12DeviceContext::InitDevice(int index)
{
	DeviceIndex = index;
	//#SLI Mask needs to be set correctly to handle mixed SLI 


#if 0
	pDXGIAdapter->RegisterVideoMemoryBudgetChangeNotificationEvent(m_VideoMemoryBudgetChange, &m_BudgetNotificationCookie);
#endif
	HeapManager = new DescriptorHeapManager(this);
	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.NodeMask = GetNodeMask();
	ThrowIfFailed(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_MainCommandQueue)));
	DEVICE_NAME_OBJECT(m_MainCommandQueue);
	for (int i = 0; i < RHI::CPUFrameCount; i++)
	{
		ThrowIfFailed(m_Device->CreateCommandAllocator(queueDesc.Type, IID_PPV_ARGS(&m_commandAllocator[i])));
		DEVICE_NAME_OBJECT(m_commandAllocator[i]);
	}
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
	ThrowIfFailed(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_ComputeCommandQueue)));

	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
	ThrowIfFailed(m_Device->CreateCommandAllocator(queueDesc.Type, IID_PPV_ARGS(&m_CopyCommandAllocator)));
	ThrowIfFailed(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CopyCommandQueue)));
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
	ThrowIfFailed(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_SharedCopyCommandQueue)));

	DEVICE_NAME_OBJECT(m_SharedCopyCommandQueue);
	DEVICE_NAME_OBJECT(m_ComputeCommandQueue);
	DEVICE_NAME_OBJECT(m_CopyCommandQueue);
	DEVICE_NAME_OBJECT(m_CopyCommandAllocator);
	for (int i = 0; i < RHI::CPUFrameCount; i++)
	{
		ThrowIfFailed(m_Device->CreateCommandAllocator(queueDesc.Type, IID_PPV_ARGS(&m_SharedCopyCommandAllocator[i])));
	}
	ThrowIfFailed(m_Device->CreateCommandList(0, queueDesc.Type, m_SharedCopyCommandAllocator[0], nullptr, IID_PPV_ARGS(&m_IntraCopyList)));
	m_IntraCopyList->Close();

	GraphicsQueueSync.Init(GetDevice());
	CopyQueueSync.Init(GetDevice());
	ComputeQueueSync.Init(GetDevice());
	GpuWaitSyncPoint.InitGPUOnly(GetDevice());

	TimeManager = new D3D12TimeManager(this);
	GPUCopyList = new D3D12CommandList(this, ECommandListType::Copy);
	InterGPUCopyList = new D3D12CommandList(this, ECommandListType::Copy);
	((D3D12CommandList*)GPUCopyList)->CreateCommandList();
	GPUCopyList->ResetList();
	GraphicsSync.Init(GetCommandQueueFromEnum(DeviceContextQueue::Graphics), GetDevice());
	CopySync.Init(GetCommandQueueFromEnum(DeviceContextQueue::Copy), GetDevice());
	InterGPUSync.Init(GetCommandQueueFromEnum(DeviceContextQueue::InterCopy), GetDevice());
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
		GetDevice()->SetStablePowerState(true);
	}
	TimeStampHeap = new D3D12QueryHeap(this, 8192, D3D12_QUERY_HEAP_TYPE_TIMESTAMP);
	CopyTimeStampHeap = new D3D12QueryHeap(this, 512, D3D12_QUERY_HEAP_TYPE_COPY_QUEUE_TIMESTAMP);
	InitCopyListPool();
	PostInit();
}

void D3D12DeviceContext::CreateNodeDevice(ID3D12Device* dev, int nodemask, int index)
{
	m_Device = dev;
	Log::LogMessage("Creating Node device " + std::to_string(nodemask) + "/" + std::to_string(m_Device->GetNodeCount()) + " Nodes");
	SetNodeMaskFromIndex(nodemask);
	InitDevice(index);
}

void D3D12DeviceContext::CreateDeviceFromAdaptor(IDXGIAdapter1 * adapter, int index)
{
	//EnableStablePower.SetValue(true);
	pDXGIAdapter = (IDXGIAdapter3*)adapter;
	pDXGIAdapter->GetDesc1(&Adaptordesc);
	VendorID = Adaptordesc.VendorId;


	HRESULT result = D3D12CreateDevice(
		pDXGIAdapter,
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&m_Device)
	);
	ensureFatalMsgf(!(result == DXGI_ERROR_UNSUPPORTED), "D3D_FEATURE_LEVEL_11_0 is required to run this engine");
	ThrowIfFailed(result);
	//ThrowIfFailed(D3D12EnableExperimentalFeatures(1, &D3D12ExperimentalShaderModels, NULL, 0));
	D3D_FEATURE_LEVEL MaxLevel = D3D12RHI::GetMaxSupportedFeatureLevel(m_Device);
	if (MaxLevel != D3D_FEATURE_LEVEL_11_0)
	{
		m_Device->Release();
		ThrowIfFailed(D3D12CreateDevice(
			pDXGIAdapter,
			MaxLevel,
			IID_PPV_ARGS(&m_Device)
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
	m_Device->QueryInterface(IID_PPV_ARGS(&m_Device2));
	m_Device->QueryInterface(IID_PPV_ARGS(&m_Device5));
	CheckFeatures();
	SetNodeMaskFromIndex(0);
	InitDevice(index);

}

void D3D12DeviceContext::LinkAdaptors(D3D12DeviceContext* other)
{
	CrossAdaptorSync[0].Init(GetDevice(), other->GetDevice());
	CrossAdaptorSync[1].Init(GetDevice(), other->GetDevice());
}

ID3D12Device * D3D12DeviceContext::GetDevice()
{
	return m_Device;
}

ID3D12Device2* D3D12DeviceContext::GetDevice2()
{
	return m_Device2;
}

ID3D12Device5 * D3D12DeviceContext::GetDevice5()
{
	return m_Device5;
}

ID3D12CommandAllocator * D3D12DeviceContext::GetCommandAllocator()
{
	return m_commandAllocator[CurrentFrameIndex];
}

ID3D12CommandAllocator * D3D12DeviceContext::GetComputeCommandAllocator()
{
	return nullptr;
}

ID3D12CommandAllocator * D3D12DeviceContext::GetCopyCommandAllocator()
{
	return nullptr;
}
ID3D12CommandAllocator * D3D12DeviceContext::GetSharedCommandAllocator()
{
	return m_SharedCopyCommandAllocator[CurrentFrameIndex];
}

ID3D12CommandQueue * D3D12DeviceContext::GetCommandQueue()
{
	return m_MainCommandQueue;
}

void D3D12DeviceContext::MoveNextFrame(int SyncIndex)
{
	//force Queue sync at the end of frame
	if (!AllowCrossFrameAsyncCompute)
	{
		InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::Compute);
		InsertGPUWait(DeviceContextQueue::Compute, DeviceContextQueue::Graphics);
	}
#if 0
	InsertGPUWait(DeviceContextQueue::InterCopy, DeviceContextQueue::Graphics);
	InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::InterCopy);
#endif
	GraphicsSync.MoveNextFrame(SyncIndex);
	CopySync.MoveNextFrame(SyncIndex);
	InterGPUSync.MoveNextFrame(SyncIndex);
	ComputeSync.MoveNextFrame(SyncIndex);
	CurrentFrameIndex = SyncIndex;

	}

void D3D12DeviceContext::ResetDeviceAtEndOfFrame()
{
	DeviceContext::ResetDeviceAtEndOfFrame();
	HeapManager->EndOfFrame();
	GetCommandAllocator()->Reset();
	GetSharedCommandAllocator()->Reset();
	ResetCopyEngine();
	//compute work could run past the end of a frame?
}

void D3D12DeviceContext::SampleVideoMemoryInfo()
{
	pDXGIAdapter->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &CurrentVideoMemoryInfo);

	usedVRAM = CurrentVideoMemoryInfo.CurrentUsage / 1024 / 1024;
	totalVRAM = CurrentVideoMemoryInfo.Budget / 1024 / 1024;
}

std::string D3D12DeviceContext::GetMemoryReport()
{
	std::string output = "VMEM: ";
	output.append(std::to_string(usedVRAM));
	output.append("MB / ");
	output.append(std::to_string(totalVRAM));
	output.append("MB");
	return output;
}

void D3D12DeviceContext::DestoryDevice()
{
	DeviceContext::DestoryDevice();
	RHI::FlushDeferredDeleteQueue();
}

void D3D12DeviceContext::WaitForGpu()
{
	GraphicsQueueSync.CreateSyncPoint(m_MainCommandQueue);
}

void D3D12DeviceContext::WaitForCopy()
{
	CopyQueueSync.CreateSyncPoint(m_SharedCopyCommandQueue);
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

ID3D12GraphicsCommandList * D3D12DeviceContext::GetCopyList()
{
	return ((D3D12CommandList*)GPUCopyList)->GetCommandList();
}

ID3D12GraphicsCommandList * D3D12DeviceContext::GetSharedCopyList()
{
	return m_IntraCopyList;
}

void D3D12DeviceContext::ResetSharingCopyList()
{
	ThrowIfFailed(m_IntraCopyList->Reset(GetSharedCommandAllocator(), nullptr));
}

void D3D12DeviceContext::NotifyWorkForCopyEngine()
{
	CopyEngineHasWork = true;
}

void D3D12DeviceContext::UpdateCopyEngine()
{
	if (true)
	{
		//CopyEngineHasWork = false;
		GPUCopyList->Execute();
	}

}

void D3D12DeviceContext::ResetCopyEngine()
{
	if (/*CopyEngineHasWork*/true)
	{
		GPUCopyList->ResetList();
	}

}

void D3D12DeviceContext::ExecuteComputeCommandList(ID3D12GraphicsCommandList * list)
{
	ID3D12CommandList* ppCommandLists[] = { list };
	m_ComputeCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	if (RHI::BlockCommandlistExec())
	{
		ComputeQueueSync.CreateSyncPoint(m_ComputeCommandQueue);
	}
}

void D3D12DeviceContext::ExecuteCopyCommandList(ID3D12GraphicsCommandList * list)
{
	ID3D12CommandList* ppCommandLists[] = { list };
	m_CopyCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	if (RHI::BlockCommandlistExec())
	{
		CopyQueueSync.CreateSyncPoint(m_CopyCommandQueue);
	}
}

void D3D12DeviceContext::ExecuteInterGPUCopyCommandList(ID3D12GraphicsCommandList * list, bool forceblock)
{
	ID3D12CommandList* ppCommandLists[] = { list };
	m_SharedCopyCommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	if (RHI::BlockCommandlistExec() || forceblock)
	{
		GraphicsQueueSync.CreateSyncPoint(m_SharedCopyCommandQueue);
	}

}
void D3D12DeviceContext::ExecuteCommandList(ID3D12GraphicsCommandList * list)
{
	ID3D12CommandList* ppCommandLists[] = { list };
	GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	if (RHI::BlockCommandlistExec())
	{
		WaitForGpu();
	}
}

RHITimeManager * D3D12DeviceContext::GetTimeManager()
{
	return TimeManager;
}

void D3D12DeviceContext::GPUWaitForOtherGPU(DeviceContext * OtherGPU, DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue)
{
	CrossAdaptorSync[GetCpuFrameIndex()].CrossGPUCreateSyncPoint(GetCommandQueueFromEnum(SignalQueue), D3D12RHI::DXConv(OtherGPU)->GetCommandQueueFromEnum(WaitingQueue));
}

bool D3D12DeviceContext::SupportsCommandList4()
{
	return SupportsCmdsList4;
}

void D3D12DeviceContext::CPUWaitForAll()
{
	GraphicsQueueSync.CreateSyncPoint(m_MainCommandQueue);
	CopyQueueSync.CreateSyncPoint(m_SharedCopyCommandQueue);
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
	GPUWaitPoints[0][SignalQueue].GPUCreateSyncPoint(GetCommandQueueFromEnum(SignalQueue), GetCommandQueueFromEnum(WaitingQueue));
}

void D3D12DeviceContext::ResetWork()
{
	CopyEngineHasWork = false;
}

RHICommandList * D3D12DeviceContext::GetInterGPUCopyList()
{
	return InterGPUCopyList;
}

DescriptorHeapManager * D3D12DeviceContext::GetHeapManager()
{
	return HeapManager;
}

D3D12QueryHeap * D3D12DeviceContext::GetTimeStampHeap()
{
	return TimeStampHeap;
}

D3D12QueryHeap * D3D12DeviceContext::GetCopyTimeStampHeap()
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

void GPUSyncPoint::Init(ID3D12Device * device, ID3D12Device* SecondDevice)
{
	//Fence types
	//  D3D12_FENCE_FLAG_NONE
	//  D3D12_FENCE_FLAG_SHARED
	//	D3D12_FENCE_FLAG_SHARED_CROSS_ADAPTER
	//	D3D12_FENCE_FLAG_NON_MONITORED
	ThrowIfFailed(device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_SHARED | D3D12_FENCE_FLAG_SHARED_CROSS_ADAPTER, IID_PPV_ARGS(&m_fence)));
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
	SecondDevice->OpenSharedHandle(fenceHandle, IID_PPV_ARGS(&secondaryFence));

	CloseHandle(fenceHandle);
}
void GPUSyncPoint::InitGPUOnly(ID3D12Device * device)
{
	ThrowIfFailed(device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	m_fenceValue++;
}
void GPUSyncPoint::Init(ID3D12Device * device)
{
	//Fence types
	//  D3D12_FENCE_FLAG_NONE
	//  D3D12_FENCE_FLAG_SHARED
	//	D3D12_FENCE_FLAG_SHARED_CROSS_ADAPTER
	//	D3D12_FENCE_FLAG_NON_MONITORED
	ThrowIfFailed(device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	m_fenceEvent = CreateEvent(nullptr, false, false, nullptr);
	if (m_fenceEvent == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}
	m_fenceValue++;
}

void GPUSyncPoint::CreateSyncPoint(ID3D12CommandQueue * queue)
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
void GPUSyncPoint::CrossGPUCreateSyncPoint(ID3D12CommandQueue * queue, ID3D12CommandQueue* otherDeviceQeue)
{
	// Schedule a Signal command in the queue.
	ThrowIfFailed(queue->Signal(m_fence, m_fenceValue));

	otherDeviceQeue->Wait(secondaryFence, m_fenceValue);
	m_fenceValue++;
}
void GPUSyncPoint::GPUCreateSyncPoint(ID3D12CommandQueue * queue, ID3D12CommandQueue * targetqueue)
{
	//Breaks!
	// Schedule a Signal command in the queue.
	ThrowIfFailed(queue->Signal(m_fence, m_fenceValue));

	targetqueue->Wait(m_fence, m_fenceValue);
	m_fenceValue++;
}

void GPUSyncPoint::Signal(ID3D12CommandQueue * queue, int value)
{
	if (value != -1)
	{
		m_fenceValue = value;
	}
	ThrowIfFailed(queue->Signal(m_fence, m_fenceValue));
}

void GPUSyncPoint::Wait(ID3D12CommandQueue * queue, int value)
{
	if (value != -1)
	{
		m_fenceValue = value;
	}
	ThrowIfFailed(queue->Wait(m_fence, m_fenceValue));
	m_fenceValue++;
}

void GPUFenceSync::Init(ID3D12CommandQueue * TargetQueue, ID3D12Device* device)
{
	Queue = TargetQueue;
	ThrowIfFailed(device->CreateFence(m_fenceValues[m_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
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
CreateChecker(D3D12GPUSyncEvent);
D3D12GPUSyncEvent::D3D12GPUSyncEvent(DeviceContextQueue::Type WaitingQueueEnum, DeviceContextQueue::Type SignalQueueEnum, DeviceContext * device, DeviceContext* OtherDevice) :RHIGPUSyncEvent(WaitingQueueEnum, SignalQueueEnum, device)
{
	AddCheckerRef(D3D12GPUSyncEvent, this);
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
{
	RemoveCheckerRef(D3D12GPUSyncEvent, this);
}
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
