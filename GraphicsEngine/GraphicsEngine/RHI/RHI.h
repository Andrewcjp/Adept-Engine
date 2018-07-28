#pragma once
#include "EngineGlobals.h"

#include "Rendering/Core/FrameBuffer.h"
#include "RHICommandList.h"
#include "Core/Assets/MeshLoader.h"
#include "RHITypes.h"
#include "Core/Assets/AssetManager.h"
#include "Rendering/Renderers/RenderSettings.h"
#include "Core/Utils/MemoryUtils.h"
//todo: refactor!
//like seriuously this is bad!
class BaseTexture;
class Renderable;
class ShaderProgramBase;
class FrameBuffer;
class DeviceContext;
const int MAX_DEVICE_COUNT = 2;
#define RHI_API DLLEXPORT
#define RHI_USE_MODULE 0

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
	CORE_API static BaseTexture* CreateTexture(AssetPathRef, DeviceContext* Device = nullptr);
	CORE_API static BaseTexture* CreateTextureWithData(int with, int height, int nChannels, void * data, DeviceContext* Device = nullptr);
	CORE_API static BaseTexture* CreateNullTexture(DeviceContext* Device = nullptr);
	CORE_API static Renderable * CreateMesh(const char * path);
	CORE_API static Renderable * CreateMesh(const char * path, MeshLoader::FMeshLoadingSettings& Settings);
	static FrameBuffer* CreateFrameBuffer(DeviceContext* Device, RHIFrameBufferDesc& Desc);
	static ShaderProgramBase* CreateShaderProgam(DeviceContext* Device = nullptr);
	static RHITextureArray * CreateTextureArray(DeviceContext * Device, int Length);
	static RHIBuffer* CreateRHIBuffer(RHIBuffer::BufferType type, DeviceContext* Device = nullptr);
	static RHIUAV* CreateUAV(DeviceContext* Device = nullptr);
	static RHICommandList* CreateCommandList(ECommandListType::Type Type = ECommandListType::Graphics, DeviceContext* Device = nullptr);

	static DeviceContext * GetDeviceContext(int index = 0);
	static DeviceContext* GetDefaultDevice();
	static bool InitialiseContext( int w, int h);
	static void RHISwapBuffers();
	static void RHIRunFirstFrame();
	static void ToggleFullScreenState();
	static void ResizeSwapChain(int width, int height);
	static void DestoryContext();

	static const MultiGPUMode* GetMGPUMode();
	static bool BlockCommandlistExec();
	static bool AllowCPUAhead();
	static int GetDeviceCount();
	static bool UseAdditonalGPUs();
	static bool IsD3D12();
	static bool SupportsThreading();
	static bool SupportsExplictMultiAdaptor();
	static ERenderSystemType GetType();
	static class RHIClass* GetRHIClass();
	static void WaitForGPU();
	static RenderConstants* GetRenderConstants();
private:
	static RHI* instance;
	ERenderSystemType CurrentSystem;
	static MultiGPUMode CurrentMGPUMode;
	class RHIClass* CurrentRHI = nullptr;
	RenderConstants M_RenderConsants;
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
	virtual void WaitForGPU() =0;
	virtual void TriggerBackBufferScreenShot() = 0;	
};
