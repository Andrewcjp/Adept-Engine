#pragma once
#include "Core/Assets/AssetTypes.h"
#include "Core/Assets/MeshLoader.h"
#include "Core/EngineInc_fwd.h"
#include "Core/Module/ModuleInterface.h"
#include "Core/Utils/MemoryUtils.h"
#include "Rendering/Renderers/RenderSettings.h"
#include "RHI_inc_fwd.h"
#include "RHITypes.h"

#define MAX_GPU_DEVICE_COUNT 2
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
	RHI_API static BaseTexture* CreateTexture(AssetPathRef, DeviceContext* Device = nullptr);
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
	static void InitialiseContext();
	static void InitialiseContextWindow(int w, int h);
	static std::string ReportMemory();
	static void RHISwapBuffers();
	static void RHIRunFirstFrame();
	static void SetFullScreenState(bool state);
	static void ToggleFullScreenState();
	static void ResizeSwapChain(int width, int height);
	static void DestoryContext();


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
	static void AddLinkedFrameBuffer(FrameBuffer* target);
	static RenderSettings* GetRenderSettings();
	static const MultiGPUMode* GetMGPUMode();
	RHI_API static void AddToDeferredDeleteQueue(IRHIResourse* Resource);
	static RHI* Get();
private:
	void TickDeferredDeleteQueue(bool Flush = false);
	static RHI* instance;
	ERenderSystemType CurrentSystem;
	static MultiGPUMode CurrentMGPUMode;
	class RHIClass* CurrentRHI = nullptr;
	RenderConstants M_RenderConsants;
	RenderSettings RenderSettings = {};
	std::vector<FrameBuffer*> FrameBuffersLinkedToSwapChain;
	int PresentCount = 0;
	typedef std::pair<IRHIResourse*, int64_t> RHIResourseStamped;
	std::vector<RHIResourseStamped> DeferredDeleteQueue;
	bool IsFlushingDeleteQueue = false;
	bool IsFullScreen = false;
};

class RHI_API RHIClass
{
public:
	RHI_VIRTUAL bool InitRHI() = 0;
	RHI_VIRTUAL bool InitWindow(int w, int h) = 0;
	RHI_VIRTUAL bool DestoryRHI() = 0;
	RHI_VIRTUAL BaseTexture* CreateTexture(DeviceContext* Device = nullptr) = 0;
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
};

class RHIModule : public IModuleInterface
{
public:
	virtual RHIClass* GetRHIClass() = 0;
};