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
	static BaseTexture* CreateTexture(const char * path, DeviceContext* Device = nullptr);
	static BaseTexture* CreateTextureWithData(int with, int height,int nChannels, void * data, DeviceContext* Device = nullptr);
	static Renderable * CreateMesh(const char * path);
	static Renderable * CreateMesh(const char * path, MeshLoader::FMeshLoadingSettings& Settings);
	static FrameBuffer* CreateFrameBuffer(int width, int height, DeviceContext* Device = nullptr, float ratio = 1.0f, FrameBuffer::FrameBufferType type = FrameBuffer::FrameBufferType::ColourDepth, glm::vec4 clearcolour = glm::vec4(0.0f, 0.2f, 0.4f, 1.0f));
	static DeviceContext * GetDeviceContext(int index);
	static ShaderProgramBase* CreateShaderProgam(DeviceContext* Device = nullptr);

	static void BindScreenRenderTarget(int mwidth, int height);
	static bool InitialiseContext(HWND m_hwnd, int w, int h);
	static void RHISwapBuffers();
	static void DestoryContext(HWND hwnd);
	static ERenderSystemType GetType();
	static bool IsOpenGL();
	static bool IsD3D12();
	static bool SupportsThreading();
	bool SupportsExplictMultiAdaptor();
	static RHIBuffer* CreateRHIBuffer(RHIBuffer::BufferType type, DeviceContext* Device = nullptr);
	static RHICommandList* CreateCommandList(DeviceContext* Device = nullptr);
	HINSTANCE m_hinst;
private:

	ERenderSystemType currentsystem;
	
	//------------------------------------
	class D3D12RHI* D3D12Rhi = nullptr;
	class VKanRHI* VulkanRHI = nullptr;
	
};

