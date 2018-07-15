#pragma once
#include "EngineGlobals.h"

#include "Rendering/Core/FrameBuffer.h"
#include "RHICommandList.h"
#include "Core/Assets/MeshLoader.h"
#include "RHITypes.h"
#include "Core/Assets/AssetManager.h"
#include "Rendering/Renderers/RenderSettings.h"
//todo: refactor!
//like seriuously this is bad!
class BaseTexture;
class Renderable;
class ShaderProgramBase;
class FrameBuffer;
class DeviceContext;
const int MAX_POINT_SHADOWS = 3;
const int MAX_DIRECTIONAL_SHADOWS = 1;
const int MAX_DEVICE_COUNT = 2;
class RHI
{
public:
	enum TextureType
	{
		Normal,
		Text
	};
	RHI();
	~RHI();
	static RHI* instance;
	static const int CPUFrameCount = 2;
	static void InitRHI(ERenderSystemType e);
	static void DestoryRHI();
	CORE_API static BaseTexture* CreateTexture(AssetManager::AssetPathRef, DeviceContext* Device = nullptr);
	CORE_API static BaseTexture* CreateTextureWithData(int with, int height,int nChannels, void * data, DeviceContext* Device = nullptr);
	CORE_API static BaseTexture* CreateNullTexture( DeviceContext* Device = nullptr);
	CORE_API static Renderable * CreateMesh(const char * path);
	CORE_API static Renderable * CreateMesh(const char * path, MeshLoader::FMeshLoadingSettings& Settings);
	static FrameBuffer* CreateFrameBuffer(DeviceContext* Device, RHIFrameBufferDesc& Desc);
	static DeviceContext * GetDeviceContext(int index);
	static ShaderProgramBase* CreateShaderProgam(DeviceContext* Device = nullptr);
	static bool InitialiseContext(HWND m_hwnd, int w, int h);
	static void RHISwapBuffers();
	static void RHIRunFirstFrame();
	static void ToggleFullScreenState();
	static void ResizeSwapChain(int width, int height);
	static void DestoryContext(HWND hwnd);
	static RHITextureArray * CreateTextureArray(DeviceContext * Device, int Length);
	static ERenderSystemType GetType();
	static bool IsOpenGL();
	static bool IsD3D12();
	static bool SupportsThreading();
	static bool SupportsExplictMultiAdaptor();
	static RHIBuffer* CreateRHIBuffer(RHIBuffer::BufferType type, DeviceContext* Device = nullptr);
	static RHIUAV* CreateUAV(DeviceContext* Device = nullptr);
	static RHICommandList* CreateCommandList(ECommandListType::Type Type = ECommandListType::Graphics,DeviceContext* Device = nullptr);
	HINSTANCE m_hinst;
	static bool BlockCommandlistExec();
	static bool AllowCPUAhead();
	static int GetDeviceCount();
	static bool UseAdditonalGPUs();
	static const MultiGPUMode* GetMGPUMode();
private: 
	ERenderSystemType CurrentSystem;
	static MultiGPUMode CurrentMGPUMode;
	//------------------------------------
	class D3D12RHI* D3D12Rhi = nullptr;
	class VKanRHI* VulkanRHI = nullptr;
	
};

