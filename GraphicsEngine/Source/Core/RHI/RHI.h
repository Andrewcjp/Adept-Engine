#pragma once

#include "Core/EngineInc_fwd.h"
#include "RHI_inc_fwd.h"
#include "RHITypes.h"
#include "Core/Assets/AssetTypes.h"
#include "Rendering/Renderers/RenderSettings.h"
#include "Core/Utils/MemoryUtils.h"
#include "Core/Assets/MeshLoader.h"
#include "RHICommandList.h"
#include "Core/Module/ModuleInterface.h"
#include "Rendering/Renderers/RenderSettings.h"

class BaseTexture;
class Renderable;
class ShaderProgramBase;
class FrameBuffer;
class DeviceContext;
const int MAX_DEVICE_COUNT = 2;
#define RHI_USE_MODULE 1
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
	RHI_API static Renderable * CreateMesh(const char * path);
	RHI_API static Renderable * CreateMesh(const char * path, MeshLoader::FMeshLoadingSettings& Settings);
	RHI_API static FrameBuffer* CreateFrameBuffer(DeviceContext* Device, RHIFrameBufferDesc& Desc);
	RHI_API static ShaderProgramBase* CreateShaderProgam(DeviceContext* Device = nullptr);
	RHI_API static RHITextureArray * CreateTextureArray(DeviceContext * Device, int Length);
	RHI_API static RHIBuffer* CreateRHIBuffer(RHIBuffer::BufferType type, DeviceContext* Device = nullptr);
	RHI_API static RHIUAV* CreateUAV(DeviceContext* Device = nullptr);
	RHI_API static RHICommandList* CreateCommandList(ECommandListType::Type Type = ECommandListType::Graphics, DeviceContext* Device = nullptr);

	RHI_API static DeviceContext * GetDeviceContext(int index = 0);
	RHI_API static DeviceContext* GetDefaultDevice();
	static void InitialiseContext(int w, int h);
	static void RHISwapBuffers();
	static void RHIRunFirstFrame();
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
	static RenderConstants* GetRenderConstants();
	static void AddLinkedFrameBuffer(FrameBuffer* target);
	static RenderSettings* GetRenderSettings();
	static const MultiGPUMode* GetMGPUMode();
private:
	static RHI* instance;
	ERenderSystemType CurrentSystem;
	static MultiGPUMode CurrentMGPUMode;
	class RHIClass* CurrentRHI = nullptr;
	RenderConstants M_RenderConsants;
	RenderSettings RenderSettings = {};
	std::vector<FrameBuffer*> FrameBuffersLinkedToSwapChain;
	int PresentCount = 0;
};

class RHI_API RHIClass
{
public:
	virtual bool InitRHI(int w, int h) = 0;
	virtual bool DestoryRHI() = 0;
	virtual BaseTexture* CreateTexture(DeviceContext* Device = nullptr) = 0;
	virtual FrameBuffer* CreateFrameBuffer(DeviceContext* Device, RHIFrameBufferDesc& Desc) = 0;
	virtual ShaderProgramBase* CreateShaderProgam(DeviceContext* Device = nullptr) = 0;
	virtual RHITextureArray * CreateTextureArray(DeviceContext * Device, int Length) = 0;
	virtual RHIBuffer* CreateRHIBuffer(RHIBuffer::BufferType type, DeviceContext* Device = nullptr) = 0;
	virtual RHIUAV* CreateUAV(DeviceContext* Device = nullptr) = 0;
	virtual RHICommandList* CreateCommandList(ECommandListType::Type Type = ECommandListType::Graphics, DeviceContext* Device = nullptr) = 0;
	virtual DeviceContext* GetDefaultDevice() = 0;
	virtual DeviceContext* GetDeviceContext(int index = 0) = 0;
	virtual void RHISwapBuffers() = 0;
	virtual void RHIRunFirstFrame() = 0;
	virtual void ToggleFullScreenState() = 0;
	virtual void ResizeSwapChain(int width, int height) = 0;
	virtual void WaitForGPU() = 0;
	virtual void TriggerBackBufferScreenShot() = 0;
};

class RHIModule : public IModuleInterface
{
public:
	virtual RHIClass* GetRHIClass() = 0;
};