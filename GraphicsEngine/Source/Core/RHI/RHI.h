#pragma once
#include "Core/Assets/AssetTypes.h"
#include "Core/Assets/MeshLoader.h"
#include "Core/EngineInc_fwd.h"
#include "Core/Module/ModuleInterface.h"
#include "Core/Utils/MemoryUtils.h"
#include "Rendering/Renderers/RenderSettings.h"
#include "RHI_inc_fwd.h"
#include "RHITypes.h"

class RHIGPUSyncEvent;
class SFRController;
class RHIClass;
#define PSO_USE_FULL_STRING_MAPS 1
#define MAX_GPU_DEVICE_COUNT 2
#define ALLOW_RESOURCE_CAPTURE 0
#define LOG_RESOURCE_TRANSITIONS 0
#define BASIC_RENDER_ONLY 0//BUILD_VULKAN
#define MAX_RENDERTARGETS 8
class RHI
{
public:
	enum TextureType
	{
		Normal,
		Text
	};
	RHI(ERenderSystemType system);
	~RHI();

	static const int CPUFrameCount = 2;
	static void InitRHI(ERenderSystemType e);
	static void DestoryRHI();
	RHI_API static BaseTexture* CreateTexture(AssetPathRef, DeviceContext* Device = nullptr, RHITextureDesc Desc = RHITextureDesc());
	RHI_API static BaseTexture* CreateTextureWithData(int with, int height, int nChannels, void * data, DeviceContext* Device = nullptr);
	RHI_API static BaseTexture* CreateNullTexture(DeviceContext* Device = nullptr);
	RHI_API static Mesh * CreateMesh(const char * path);
	RHI_API static Mesh * CreateMesh(const char * path, MeshLoader::FMeshLoadingSettings& Settings);
	RHI_API static FrameBuffer* CreateFrameBuffer(DeviceContext* Device, const RHIFrameBufferDesc& Desc);
	RHI_API static ShaderProgramBase* CreateShaderProgam(DeviceContext* Device = nullptr);
	RHI_API static RHITextureArray * CreateTextureArray(DeviceContext * Device, int Length);
	RHI_API static RHIBuffer* CreateRHIBuffer(ERHIBufferType::Type type, DeviceContext* Device = nullptr);
	RHI_API static RHIUAV* CreateUAV(DeviceContext* Device = nullptr);
	RHI_API static RHICommandList* CreateCommandList(ECommandListType::Type Type = ECommandListType::Graphics, DeviceContext* Device = nullptr);
	RHI_API static DeviceContext * GetDeviceContext(int index = 0);
	RHI_API static DeviceContext* GetDefaultDevice();
	RHI_API static RHIPipeLineStateObject* CreatePipelineStateObject(const RHIPipeLineStateDesc& Desc, DeviceContext* Device = nullptr);
	static void InitialiseContext();
	void ValidateSettings();
	static void InitialiseContextWindow(int w, int h);
	static std::string ReportMemory();
	static void RHISwapBuffers();
	static void RHIRunFirstFrame();
	static void SetFullScreenState(bool state);
	static void ToggleFullScreenState();
	static void ResizeSwapChain(int width, int height);
	static void DestoryContext();

	RHI_API static RHIGPUSyncEvent* CreateSyncEvent(DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue, DeviceContext * Device = nullptr, DeviceContext * SignalDevice = nullptr);
	RHI_API static bool BlockCommandlistExec();
	RHI_API static bool AllowCPUAhead();
	RHI_API static int GetDeviceCount();
	RHI_API static int GetFrameCount();

	static bool UseAdditionalGPUs();
	static bool IsD3D12();
	static bool SupportsThreading();
	static bool SupportsExplictMultiAdaptor();
	RHI_API static ERenderSystemType GetType();
	static class RHIClass* GetRHIClass();
	static void WaitForGPU();
	static const RenderConstants* GetRenderConstants();
	static void AddLinkedFrameBuffer(FrameBuffer* target, bool NoResize = false);

	static void RemoveLinkedFrameBuffer(FrameBuffer* target);
	RHI_API static RenderSettings* GetRenderSettings();
	RHI_API static const MultiGPUMode* GetMGPUSettings();
	RHI_API static void AddToDeferredDeleteQueue(IRHIResourse* Resource);
	static RHI* Get();
	static SFRController* GetSplitController();
	RHI_API static void FlushDeferredDeleteQueue();
	static void Tick();
private:
	static void ResizeFrameBuffer(FrameBuffer * target);
	SFRController* SFR_Controller = nullptr;
	void TickDeferredDeleteQueue(bool Flush = false);
	static RHI* instance;
	ERenderSystemType CurrentSystem;
	MultiGPUMode CurrentMGPUMode = MultiGPUMode();
	RHIClass* CurrentRHI = nullptr;
	RenderConstants M_RenderConsants;
	RenderSettings RenderSettings = {};
	std::vector<FrameBuffer*> FrameBuffersLinkedToSwapChain;
	int PresentCount = 0;
	typedef std::pair<IRHIResourse*, int64_t> RHIResourseStamped;
	std::vector<RHIResourseStamped> DeferredDeleteQueue;
	bool IsFlushingDeleteQueue = false;
	bool IsFullScreen = false;
	int SwapChainWidth = 0;
	int SwapChainHeight = 0;
};

class RHI_API RHIClass
{
public:
	RHI_VIRTUAL bool InitRHI() = 0;
	RHI_VIRTUAL bool InitWindow(int w, int h) = 0;
	RHI_VIRTUAL bool DestoryRHI() = 0;

	RHI_VIRTUAL BaseTexture* CreateTexture(const RHITextureDesc& Desc, DeviceContext* Device = nullptr) = 0;
	RHI_VIRTUAL FrameBuffer* CreateFrameBuffer(DeviceContext* Device, const RHIFrameBufferDesc& Desc) = 0;
	RHI_VIRTUAL ShaderProgramBase* CreateShaderProgam(DeviceContext* Device = nullptr) = 0;
	RHI_VIRTUAL RHITextureArray * CreateTextureArray(DeviceContext * Device, int Length) = 0;
	RHI_VIRTUAL RHIBuffer* CreateRHIBuffer(ERHIBufferType::Type type, DeviceContext* Device = nullptr) = 0;
	RHI_VIRTUAL RHIUAV* CreateUAV(DeviceContext* Device = nullptr) = 0;
	RHI_VIRTUAL RHICommandList* CreateCommandList(ECommandListType::Type Type = ECommandListType::Graphics, DeviceContext* Device = nullptr) = 0;
	RHI_VIRTUAL DeviceContext* GetDefaultDevice() = 0;
	RHI_VIRTUAL DeviceContext* GetDeviceContext(int index = 0) = 0;
	RHI_VIRTUAL void RHISwapBuffers() = 0;
	RHI_VIRTUAL void RHIRunFirstFrame() = 0;
	RHI_VIRTUAL void SetFullScreenState(bool state) = 0;
	RHI_VIRTUAL void ResizeSwapChain(int width, int height) = 0;
	RHI_VIRTUAL void WaitForGPU() = 0;
	RHI_VIRTUAL void TriggerBackBufferScreenShot() = 0;
	RHI_VIRTUAL std::string ReportMemory() = 0;
	RHI_VIRTUAL RHIPipeLineStateObject* CreatePSO(const RHIPipeLineStateDesc& Desc, DeviceContext * Device) = 0;
	RHI_VIRTUAL RHIGPUSyncEvent* CreateSyncEvent(DeviceContextQueue::Type WaitingQueue, DeviceContextQueue::Type SignalQueue, DeviceContext * Device, DeviceContext * SignalDevice) = 0;
#if ALLOW_RESOURCE_CAPTURE
	RHI_VIRTUAL void TriggerWriteBackResources() = 0;
#endif
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
	//uint is the hash of the pso desc.
#if PSO_USE_FULL_STRING_MAPS
	std::map<std::string, RHIPipeLineStateObject*> PSOMap;
#else
	std::map<size_t, RHIPipeLineStateObject*> PSOMap;
#endif
};