#pragma once
#include "RHI/RHI.h"
#include "CommandListPool.h"

struct EGPUType
{
	enum Type
	{
		Dedicated,
		Dedicated_Linked,
		Intergrated,
		Software,
		Limit
	};
	RHI_API static std::string ToString(EGPUType::Type type);
};
class GPUStateCache;
class GPUTextureStreamer;
class RHICommandList;
class FrameBuffer;
class RHIPipeLineStateObject;
class PipelineStateObjectCache;
class IRHIResourse;
class RHIFence;
struct ERayTracingSupportType
{
	enum Type
	{
		None,
		DriverBased,//Driver provides RT layer on GPU with no RT hardware.
		Hardware,//Full hardware acceleration support
		Limit
	};
	CORE_API static const char* ToString(ERayTracingSupportType::Type e);
};
struct EVRSSupportType
{
	enum Type
	{
		None,
		Hardware,
		Hardware_Tier2,
		Limit
	};
};
struct EMGPUConnectionMode
{
	enum Type
	{
		None,
		HostStagedTransfer,
		DirectTransfer,
		AcceleratedDirectTransfer,
		Limit
	};
	CORE_API static const char* ToString(EMGPUConnectionMode::Type e);
};
struct EShaderSupportModel
{
	enum Type
	{
		SM5,
		SM6,
		Limit
	};
	CORE_API static const char* ToString(EShaderSupportModel::Type e);
};
struct CapabilityData
{
	bool SupportsCopyTimeStamps = false;
	bool SupportsViewInstancing = false;
	ERayTracingSupportType::Type RTSupport = ERayTracingSupportType::None;
	EVRSSupportType::Type VRSSupport = EVRSSupportType::None;
	//If driver supports multiple SLI group this will fail to use the HW fully.
	EMGPUConnectionMode::Type ConnectionMode = EMGPUConnectionMode::None;
	bool SupportsDepthBoundsTest = false;
	EShaderSupportModel::Type HighestModel = EShaderSupportModel::SM5;
	bool SupportExecuteIndirect = false;
	int VRSTileSize = 16;

	bool SupportsConservativeRaster = false;

	bool SupportTypedUAVLoads = false;
};
const int COPYLIST_MAX_POOL_SIZE = 4;
class  DeviceContext
{
public:
	RHI_API DeviceContext();
	RHI_API virtual ~DeviceContext();

	RHI_API virtual void ResetDeviceAtEndOfFrame() = 0;
	RHI_API virtual void SampleVideoMemoryInfo() = 0;
	RHI_API virtual RHIClass::GPUMemoryData GetMemoryReport() = 0;
	RHI_API virtual void DestoryDevice() = 0;
	RHI_API virtual void WaitForGpu() = 0;
	RHI_API virtual void WaitForCopy() = 0;
	RHI_API virtual void NotifyWorkForCopyEngine() = 0;
	RHI_API virtual void UpdateCopyEngine() = 0;
	RHI_API virtual void ResetCopyEngine() = 0;

	RHI_API int GetDeviceIndex() const;
	RHI_API int GetCpuFrameIndex() const;
	RHI_API virtual void GPUWaitForOtherGPU(DeviceContext* OtherGPU, DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue) = 0;

	RHI_API virtual void CPUWaitForAll() = 0;
	RHI_API virtual void InsertGPUWait(DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue) = 0;
	RHI_API virtual void InsertCrossGPUWait(DeviceContextQueue::Type WaitingQueue, DeviceContext* SignalingGPU, DeviceContextQueue::Type SignalQueue) = 0;
	RHI_API const CapabilityData& GetCaps();
	RHI_API RHICommandList* GetInterGPUCopyList();
	RHI_API virtual class RHITimeManager* GetTimeManager() = 0;
	RHI_API void InsertStallTimerMarker();
	RHI_API bool ShouldInsertTimer();
	RHI_API void OnInsertStallTimer();
	RHI_API PipelineStateObjectCache* GetPSOCache() const;
	RHI_API void UpdatePSOTracker(RHIPipeLineStateObject* PSO);
	RHI_API int GetNodeIndex();
	RHI_API GPUStateCache* GetStateCache()const;
	RHI_API bool IsDeviceNVIDIA();
	RHI_API bool IsDeviceAMD();
	RHI_API bool IsDeviceIntel();
	//Copy lists are pooled (because why not)
	RHI_API RHICommandList* GetCopyList(int Index);
	RHI_API RHICommandList* GetNextFreeCopyList();
	RHI_API void TickTransferStats(bool render);
	RHI_API int GetTransferBytes();
	RHI_API void AddTransferBuffer(FrameBuffer* buffer);
	RHI_API void RemoveTransferBuffer(FrameBuffer* buffer);
	void ResetStat();
	RHI_API uint GetNodeMask();
	RHI_API void SetNodeMaskFromIndex(int GPUIndex);
	RHI_API bool IsDedicated();
	RHI_API bool IsIntergrated();
	RHI_API EGPUType::Type GetType();

	//events
	RHI_API virtual void OnFrameStart();

	GPUTextureStreamer* GetStreamer();
	RHI_API void IncrementDeviceFrame();
	RHI_API int GetDeviceFrame()const;
	void TickDeferredDeleteQueue();
	bool SupportsIndirectExecute()const;
	CommandListPool* GetListPool() { return &Pool; }
	RHI_API virtual bool IsUMA()const = 0;
	RHI_API int SignalCommandQueue(DeviceContextQueue::Type queue, uint64 value = -1);
	RHI_API int InsertWaitForValue(DeviceContextQueue::Type queue, DeviceContextQueue::Type WaitingQueue, uint64 value = -1);
protected:
	int CurrentFrameIndex = 0;
	RHI_API void PostInit();
	bool AllowCrossFrameAsyncCompute = false;
	RHI_API void InitCopyListPool();
	bool LogDeviceDebug = true;
	int DeviceIndex = 0;
	bool InsertStallTimer = false;
	//copy queue management 
	bool CopyEngineHasWork = false;
	RHICommandList* InterGPUCopyList = nullptr;
	RHICommandList* GPUCopyList = nullptr;
	CapabilityData Caps_Data = CapabilityData();
	PipelineStateObjectCache* PSOCache = nullptr;
	RHIPipeLineStateObject* CurrentGPUPSO = nullptr;
	//For creating a fake device in Linked adapter mode
	int NodeIndex = 0;
	GPUStateCache* StateCache = nullptr;
	uint VendorID = 0;
	int CopyListPoolFreeIndex = 0;
	RHICommandList* CopyListPool[COPYLIST_MAX_POOL_SIZE] = { nullptr };
	int BytesToTransfer = 0;
	std::vector<FrameBuffer*> BuffersWithTransfers;
	uint GPUMask = 0;
	EGPUType::Type GPUType = EGPUType::Limit;
	GPUTextureStreamer* Streamer = nullptr;
	//Devices might not move to the next frame in sync 
	int DeviceFrameNumber = 0;
	std::vector<IRHIResourse*> DeferredDeleteQueue;
	CommandListPool Pool;
	RHIFence* QueueFences[DeviceContextQueue::LIMIT] = { nullptr };
};

class RHIGPUSyncEvent : public IRHIResourse
{
public:
	RHI_API RHIGPUSyncEvent(DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue, DeviceContext* Device);
	RHI_API virtual ~RHIGPUSyncEvent();
	RHI_API virtual void Signal() = 0;
	RHI_API virtual void Wait() = 0;
	const char* GetDebugName()
	{
		return "GPUSyncEvent";
	}
protected:
	DeviceContext* Device = nullptr;
};
