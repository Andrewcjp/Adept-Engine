#pragma once
#include "Rendering\Renderers\RenderSettings.h"
#include "Rendering\VR\HMD.h"
#include "RHITypes.h"
#include "RHIQuery.h"
#include "Core\Assets\AssetTypes.h"
#include "Core\Assets\MeshLoader.h"
#include "Rendering\RayTracing\RHIStateObject.h"
#include "Core\Module\ModuleInterface.h"
#include "RHI_inc_fwd.h"
#include "RHIFence.h"
#define USE_FLAT_COMPUTE 0
#if USE_FLAT_COMPUTE
#define FLAT_COMPUTE_START(device) device->InsertGPUWait(EDeviceContextQueue::Compute, EDeviceContextQueue::Graphics);
#define FLAT_COMPUTE_END(device) device->InsertGPUWait(EDeviceContextQueue::Graphics, EDeviceContextQueue::Compute);
#else
#define FLAT_COMPUTE_START(device)
#define FLAT_COMPUTE_END(device) 
#endif
#define NOAPIIMP(func) ensureMsgf(false, #func" Needs API implmentation");
class RHIGPUSyncEvent;
class SFRController;
class RHIClass;
class HMD;
class HMDManager;
struct VRSettings;
class LowLevelAccelerationStructure;
class HighLevelAccelerationStructure;
class RayTracingEngine;
class RHIStateObject;
class RHIRenderPassCache;
class RenderGraphSystem;
class RHIInterGPUStagingResource;
class RHIRenderPass;
struct InterGPUDesc;
class GPUPerformanceTestManager;
class FrameBuffer;
struct RenderConstants;
class IRHIResourse;
struct RHIRenderPassDesc;
class DeviceContext;
class RHIQuery;
class Mesh;
class ShaderProgramBase;
class RHITextureArray;
class RHIBuffer;

class RHICommandList;
class RHITexture;
class DynamicQualityEngine;
class RHIFence;
class GPULoadBalancer;

//RHI defines
#define RHI_SUPPORTS_VR 1
#define PSO_USE_MAP 1
#define MAX_GPU_DEVICE_COUNT 3
#define ALLOW_RESOURCE_CAPTURE 0
#define LOG_RESOURCE_TRANSITIONS 0
#define MAX_RENDERTARGETS 8
#ifdef PLATFORM_RT
#define RHI_SUPPORTS_RT 1
#else
#define RHI_SUPPORTS_RT 0
#endif
class RHI
{
public:
	RHI(ERenderSystemType system);
	~RHI();

	static const int CPUFrameCount = 2;
	static void InitRHI(ERenderSystemType e);
	static void DestoryRHI();
	RHI_API static BaseTextureRef CreateTexture(AssetPathRef, DeviceContext* Device = nullptr, RHITextureDesc Desc = RHITextureDesc());
	RHI_API static BaseTexture* CreateTextureWithData(int with, int height, int nChannels, void * data, DeviceContext* Device = nullptr);
	RHI_API static BaseTexture* CreateNullTexture(DeviceContext* Device = nullptr);
	RHI_API static Mesh * CreateMesh(const char * path);
	RHI_API static Mesh * CreateMesh(const char * path, MeshLoader::FMeshLoadingSettings& Settings);
	RHI_API static FrameBuffer* CreateFrameBuffer(DeviceContext* Device, const RHIFrameBufferDesc& Desc,bool AutoSizeDesc = false);
	RHI_API static ShaderProgramBase* CreateShaderProgam(DeviceContext* Device = nullptr);
	RHI_API static RHITextureArray * CreateTextureArray(DeviceContext * Device, int Length);
	RHI_API static bool SupportsUnlinkedAdaptors();
	RHI_API static RHIBuffer* CreateRHIBuffer(ERHIBufferType::Type type, DeviceContext* Device = nullptr);
	RHI_API static RHICommandList* CreateCommandList(ECommandListType::Type Type = ECommandListType::Graphics, DeviceContext* Device = nullptr);
	RHI_API static DeviceContext * GetDeviceContext(int index = 0);
	RHI_API static DeviceContext* GetDefaultDevice();
	RHI_API static RHIPipeLineStateObject* CreatePipelineStateObject(const RHIPipeLineStateDesc& Desc, DeviceContext* Device = nullptr);
	RHI_API static RHIQuery* CreateQuery(EGPUQueryType::Type type, DeviceContext* con = nullptr);
	static void InitialiseContext();
	void ValidateSettings();
	static void InitialiseContextWindow(int w, int h);
	static std::string ReportMemory(bool all = false);
	static void RHISwapBuffers();
	static void RHIRunFirstFrame();
	static void SetFullScreenState(bool state);
	static void ToggleFullScreenState();
	static void ResizeSwapChain(int width, int height);
	static void DestoryContext();
	static void BeginFrame();
	RHI_API static RHIGPUSyncEvent* CreateSyncEvent(EDeviceContextQueue::Type WaitingQueue, EDeviceContextQueue::Type SignalQueue, DeviceContext * Device = nullptr, DeviceContext * SignalDevice = nullptr);
	RHI_API static bool BlockCommandlistExec();
	RHI_API static bool AllowCPUAhead();
	RHI_API static int GetDeviceCount();
	RHI_API static int GetFrameCount();

	//VR
	static bool SupportVR();
	void DetectAndInitVR();
	static HMD* GetHMD();
	static HMDManager* GetHMDManager();
	//is everything valid to render for vr?
	RHI_API static bool IsRenderingVR();
	//checks
	RHI_API static bool UseAdditionalGPUs();
	static bool IsD3D12();
	static bool IsNullRHIActingDX12();
	static bool IsVulkan();
	static bool SupportsThreading();
	static bool SupportsExplictMultiAdaptor();
	RHI_API static bool AllowIHVAcceleration();
	RHI_API static ERenderSystemType GetType();
	static class RHIClass* GetRHIClass();
	RHI_API static void WaitForGPU();
	static const RenderConstants* GetRenderConstants();
	static void AddLinkedFrameBuffer(FrameBuffer* target, bool NoResize = false);

	static void RemoveLinkedFrameBuffer(FrameBuffer* target);
	RHI_API static RenderSettings* GetRenderSettings();

	static VRSettings* GetVrSettings();
	RHI_API static void AddToDeferredDeleteQueue(IRHIResourse* Resource);
	static RHI* Get();
	static SFRController* GetSplitController();
	RHI_API static void FlushDeferredDeleteQueue();
	static void Tick();
	static void SubmitToVRComposter(FrameBuffer* fb, EEye::Type eye);
	RHI_API static RHIRenderPassDesc GetRenderPassDescForSwapChain(bool ClearScreen = false);
	static RenderGraphSystem* GetRenderSystem();
	static GPUPerformanceTestManager* GetTestManager();
	static void RunGPUTests();
	static void MakeSwapChainReady(RHICommandList* list);
	
private:
	static void ValidateDevice(DeviceContext*& con);
	static void ResizeFrameBuffer(FrameBuffer * target);	
	GPULoadBalancer* LoadBalencer = nullptr;
	void TickDeferredDeleteQueue(bool Flush = false);
	static RHI* instance;
	ERenderSystemType CurrentSystem;

	RHIClass* CurrentRHI = nullptr;
	RenderConstants M_RenderConsants;
	RenderSettings Rendersettings = {};
	std::vector<FrameBuffer*> FrameBuffersLinkedToSwapChain;
	int PresentCount = 0;
	typedef std::pair<IRHIResourse*, int64_t> RHIResourseStamped;
	std::vector<RHIResourseStamped> DeferredDeleteQueue;
	bool IsFlushingDeleteQueue = false;
	bool IsFullScreen = false;
	int SwapChainWidth = 0;
	int SwapChainHeight = 0;
	friend RayTracingEngine;
	RayTracingEngine* RTE = nullptr;
#if RHI_SUPPORTS_VR
	HMDManager* HeadSetManager = nullptr;
#endif
	friend RHIRenderPassCache;
	RHIRenderPassCache* RenderPassCache = nullptr;
	RenderGraphSystem* RenderSystem = nullptr;
	GPUPerformanceTestManager* TestManager = nullptr;
	FrameCountingVector<IRHIResourse>* GlobalDeleteQueue= nullptr;
	DynamicQualityEngine* QualityEngine = nullptr;
	friend DynamicQualityEngine;
	bool CheckSwapReady = false;
	ERenderSystemType RHIEmulationTarget = Limit;
};

class RHI_API RHIClass
{
public:
	struct GPUMemoryData
	{
		uint64_t TotalAllocated = 0;
		int64_t UntrackedDelta = 0;
		uint64_t MaxBudget = 0;
		uint64_t MaxPhysical = 0;
	};
	struct GPUMemoryReport
	{
		std::vector<GPUMemoryData> GpuData;
	};
	RHI_VIRTUAL bool InitRHI(bool InitAllGPUs) = 0;
	RHI_VIRTUAL bool InitWindow(int w, int h) = 0;
	RHI_VIRTUAL bool DestoryRHI() = 0;

	RHI_VIRTUAL BaseTexture* CreateTexture(const RHITextureDesc& Desc, DeviceContext* Device = nullptr) = 0;
	RHI_VIRTUAL FrameBuffer* CreateFrameBuffer(DeviceContext* Device, const RHIFrameBufferDesc& Desc) = 0;
	RHI_VIRTUAL ShaderProgramBase* CreateShaderProgam(DeviceContext* Device = nullptr) = 0;
	RHI_VIRTUAL RHITextureArray * CreateTextureArray(DeviceContext * Device, int Length) = 0;
	RHI_VIRTUAL RHIBuffer* CreateRHIBuffer(ERHIBufferType::Type type, DeviceContext* Device = nullptr) = 0;
	RHI_VIRTUAL RHICommandList* CreateCommandList(ECommandListType::Type Type = ECommandListType::Graphics, DeviceContext* Device = nullptr) = 0;
	RHI_VIRTUAL DeviceContext* GetDefaultDevice() = 0;
	RHI_VIRTUAL DeviceContext* GetDeviceContext(int index = 0) = 0;
	RHI_VIRTUAL void RHISwapBuffers() = 0;
	RHI_VIRTUAL void RHIRunFirstFrame() = 0;
	RHI_VIRTUAL void SetFullScreenState(bool state) = 0;
	RHI_VIRTUAL void ResizeSwapChain(int width, int height) = 0;
	RHI_VIRTUAL void WaitForGPU() = 0;
	RHI_VIRTUAL void TriggerBackBufferScreenShot() = 0;
	RHI_VIRTUAL GPUMemoryReport ReportMemory() = 0;
	RHI_VIRTUAL RHIPipeLineStateObject* CreatePSO(const RHIPipeLineStateDesc& Desc, DeviceContext * Device) = 0;
	RHI_VIRTUAL RHIGPUSyncEvent* CreateSyncEvent(EDeviceContextQueue::Type WaitingQueue, EDeviceContextQueue::Type SignalQueue, DeviceContext * Device, DeviceContext * SignalDevice) = 0;
#if ALLOW_RESOURCE_CAPTURE
	RHI_VIRTUAL void TriggerWriteBackResources() = 0;
#endif
	RHI_VIRTUAL void SubmitToVRComposter(FrameBuffer* fb, EEye::Type eye);
	RHI_VIRTUAL RHIQuery * CreateQuery(EGPUQueryType::Type type, DeviceContext * con) = 0;

	//RT

	RHI_VIRTUAL LowLevelAccelerationStructure* CreateLowLevelAccelerationStructure(DeviceContext * Device, const AccelerationStructureDesc & Desc);
	RHI_VIRTUAL HighLevelAccelerationStructure* CreateHighLevelAccelerationStructure(DeviceContext * Device, const AccelerationStructureDesc & Desc);
	RHI_VIRTUAL RHIStateObject* CreateStateObject(DeviceContext* Device, RHIStateObjectDesc Desc);

	RHI_VIRTUAL RHIRenderPass* CreateRenderPass(const RHIRenderPassDesc & Desc, DeviceContext* Device) = 0;
	RHI_VIRTUAL RHIRenderPassDesc GetRenderPassDescForSwapChain(bool ClearScreen = false);

	//MGPU
	RHI_VIRTUAL RHIInterGPUStagingResource* CreateInterGPUStagingResource(DeviceContext* Owner, const InterGPUDesc& desc) = 0;

	RHI_VIRTUAL RHITexture* CreateTexture2() = 0;
	//allow an RHI to add resource transitions for the swap chain to a large list for performance.
	RHI_VIRTUAL void MakeSwapChainReady(RHICommandList* list) = 0;
	RHI_VIRTUAL RHIFence* CreateFence(DeviceContext* device, EFenceFlags::Type Flags = EFenceFlags::None) = 0;
};

class RHIModule : public IModuleInterface
{
public:
	virtual RHIClass* GetRHIClass() = 0;
};

//PSOs are cached on per device basis
class PipelineStateObjectCache
{
public:
	PipelineStateObjectCache(DeviceContext* dev);
	~PipelineStateObjectCache();
	CORE_API RHIPipeLineStateObject* GetFromCache(RHIPipeLineStateDesc& desc);
	CORE_API void AddToCache(RHIPipeLineStateObject* object);
	void Destory();
private:
	DeviceContext* Device = nullptr;
	std::mutex CacheLock;
	//uint is the hash of the pso desc.
#if PSO_USE_MAP
	std::map<size_t, RHIPipeLineStateObject*> PSOMap;
#else
	std::vector<RHIPipeLineStateObject*> PSOMap;
#endif
};