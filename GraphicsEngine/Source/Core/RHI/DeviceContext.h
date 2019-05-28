#pragma once

#include "RHI/RHI_inc.h"
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
struct ERayTracingSupportType
{
	enum Type
	{
		Hardware,
		DriverBased,
		Software,
		Limit
	};
};
struct EVRSSupportType
{
	enum Type
	{
		Hardware,
		Software,
		Limit
	};
};
struct CapabilityData
{
	bool SupportsCopyTimeStamps = false;
	bool SupportsViewInstancing = false;
	ERayTracingSupportType::Type RTSupport = ERayTracingSupportType::Software;
	EVRSSupportType::Type VRSSupport = EVRSSupportType::Software;
};
const int COPYLIST_MAX_POOL_SIZE = 4;
class  DeviceContext
{
public:
	RHI_API DeviceContext();
	RHI_API virtual ~DeviceContext();

	RHI_API virtual void ResetDeviceAtEndOfFrame() = 0;
	RHI_API virtual void SampleVideoMemoryInfo() = 0;
	RHI_API virtual std::string GetMemoryReport() = 0;
	RHI_API virtual void DestoryDevice() = 0;
	RHI_API virtual void WaitForGpu() = 0;
	RHI_API virtual void WaitForCopy() = 0;
	RHI_API virtual void ResetSharingCopyList() = 0;
	RHI_API virtual void NotifyWorkForCopyEngine() = 0;
	RHI_API virtual void UpdateCopyEngine() = 0;
	RHI_API virtual void ResetCopyEngine() = 0;

	RHI_API  int GetDeviceIndex();
	RHI_API virtual int GetCpuFrameIndex() = 0;
	RHI_API virtual void GPUWaitForOtherGPU(DeviceContext * OtherGPU, DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue) = 0;
	int CurrentFrameIndex = 0;
	RHI_API virtual void CPUWaitForAll() = 0;
	RHI_API virtual void InsertGPUWait(DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue) = 0;
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
	RHI_API virtual void OnFrameEnd_PreSubmit();
protected:
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
};

class RHIGPUSyncEvent
{
public:
	RHI_API RHIGPUSyncEvent(DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue, DeviceContext * Device);
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
