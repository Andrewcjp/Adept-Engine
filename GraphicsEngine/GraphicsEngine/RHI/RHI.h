#pragma once
#include "EngineGlobals.h"
#include <Windows.h>
#include "../Rendering/Core/FrameBuffer.h"
#include "RHICommandList.h"
#include "../Core/Assets/MeshLoader.h"
//todo: refactor!
//like seriuously this is bad!
class BaseTexture;
class Renderable;
class ShaderProgramBase;
class FrameBuffer;
class DeviceContext;
const int MAX_POINT_SHADOWS = 3;
const int MAX_DIRECTIONAL_SHADOWS = 2;
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

	static void InitRHI(ERenderSystemType e);
	static void DestoryRHI();
	CORE_API static BaseTexture* CreateTexture(const char * path, DeviceContext* Device = nullptr);
	CORE_API static BaseTexture* CreateTextureWithData(int with, int height,int nChannels, void * data, DeviceContext* Device = nullptr);
	CORE_API static BaseTexture* CreateNullTexture( DeviceContext* Device = nullptr);
	CORE_API static Renderable * CreateMesh(const char * path);
	CORE_API static Renderable * CreateMesh(const char * path, MeshLoader::FMeshLoadingSettings& Settings);
	static FrameBuffer* CreateFrameBuffer(int width, int height, DeviceContext* Device = nullptr, float ratio = 1.0f, FrameBuffer::FrameBufferType type = FrameBuffer::FrameBufferType::ColourDepth, glm::vec4 clearcolour = glm::vec4(0.0f, 0.2f, 0.4f, 1.0f));
	static DeviceContext * GetDeviceContext(int index);
	static ShaderProgramBase* CreateShaderProgam(DeviceContext* Device = nullptr);
	static bool InitialiseContext(HWND m_hwnd, int w, int h);
	static void RHISwapBuffers();
	static void DestoryContext(HWND hwnd);
	static RHITextureArray * CreateTextureArray(DeviceContext * Device, int Length);
	static ERenderSystemType GetType();
	static bool IsOpenGL();
	static bool IsD3D12();
	static bool SupportsThreading();
	static bool SupportsExplictMultiAdaptor();
	static RHIBuffer* CreateRHIBuffer(RHIBuffer::BufferType type, DeviceContext* Device = nullptr);
	static RHICommandList* CreateCommandList(DeviceContext* Device = nullptr);
	HINSTANCE m_hinst;
	static bool RunRenderersAsync();
	static bool RunListsAsync();
private: 
	bool ShouldRunAsync = false;
	ERenderSystemType CurrentSystem;
	
	//------------------------------------
	class D3D12RHI* D3D12Rhi = nullptr;
	class VKanRHI* VulkanRHI = nullptr;
	
};

