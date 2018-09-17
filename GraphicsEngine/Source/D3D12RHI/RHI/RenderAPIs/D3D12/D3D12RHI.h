#pragma once
//this class is to wrap all d3d12 rhi stuff up in
//and handle object creation
//and prevent RHI.cpp from getting extremly large
#include <d3d12.h>
#include <DXGI1_4.h>
#include "d3dx12.h"
#include <vector>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
//#pragma comment(lib, "dxgidebug.lib")
#include "d3d12Shader.h"
#include "D3D12Texture.h"
#include "D3D12Helpers.h"
#include "Core/Utils/RefChecker.h"
#include "Core/Module/ModuleManager.h"
#include "Core/Platform/PlatformCore.h"
#define USEGPUTOGENMIPS_ATRUNTIME 0

class D3D12RHI : public RHIClass
{
public:
	D3D12RHI();
	~D3D12RHI();
	void SetScreenRenderTarget(ID3D12GraphicsCommandList * list);
	void RenderToScreen(ID3D12GraphicsCommandList * list);
	void ClearRenderTarget(ID3D12GraphicsCommandList * MainList);
	static D3D12RHI* Instance;
	void AddObjectToDeferredDeleteQueue(IUnknown * Target);
	static D3D12RHI* Get();
	static D3D_FEATURE_LEVEL GetMaxSupportedFeatureLevel(ID3D12Device * pDevice);
	static bool DetectGPUDebugger();
	//temp To be RHI'D	
	std::string GetMemory();
private:
	void DestroyContext();
	void PresentFrame();
	void DisplayDeviceDebug();
	void ReportObjects();
	void LoadPipeLine();
	void CreateSwapChainRTs();
	void InitMipmaps();
	void ReleaseSwapRTs();
	void CreateDepthStencil(int width, int height);
	void InitSwapChain();
	void ToggleFullScreenState();
	void ExecSetUpList();
	void ReleaseUploadHeaps(bool force = false);

	void FindAdaptors(IDXGIFactory2 * pFactory);
	void MoveToNextFrame();

	class ShaderMipMap* MipmapShader = nullptr;
	ID3D12DescriptorHeap* BaseTextureHeap;
	ID3D12CommandQueue* GetCommandQueue();
	bool InitRHI()override;
	bool InitWindow(int w, int h) override;
	bool DestoryRHI() override;
	DeviceContext* GetDefaultDevice() override;
	DeviceContext* GetDeviceContext(int index = 0) override;
	BaseTexture* CreateTexture(DeviceContext* Device = nullptr) override;
	FrameBuffer* CreateFrameBuffer(DeviceContext* Device, RHIFrameBufferDesc& Desc) override;
	ShaderProgramBase* CreateShaderProgam(DeviceContext* Device = nullptr) override;
	RHITextureArray * CreateTextureArray(DeviceContext * Device, int Length) override;
	RHIBuffer* CreateRHIBuffer(RHIBuffer::BufferType type, DeviceContext* Device = nullptr) override;
	RHIUAV* CreateUAV(DeviceContext* Device = nullptr) override;
	RHICommandList* CreateCommandList(ECommandListType::Type Type = ECommandListType::Graphics, DeviceContext* Device = nullptr)override;
	virtual void TriggerBackBufferScreenShot() override;

	void WaitForGPU() override;
	void RHISwapBuffers() override;
	void RHIRunFirstFrame() override;
	void ResizeSwapChain(int x, int y) override;

private:
	IDXGIFactory4 * factory = nullptr;
	ID3D12Device * GetDisplayDevice();
	class	D3D12DeviceContext* PrimaryDevice = nullptr;
	class	D3D12DeviceContext* SecondaryDevice = nullptr;
	void ExecList(ID3D12GraphicsCommandList * list, bool block = false);
	ID3D12GraphicsCommandList* m_SetupCommandList;
	int m_width = 0;
	int m_height = 0;
	float m_aspectRatio = 0.0f;
	bool HasSetup = false;
	bool IsFullScreen = false;
	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;
	IDXGISwapChain3* m_swapChain;
	ID3D12Resource* m_SwaprenderTargets[RHI::CPUFrameCount];
	ID3D12DescriptorHeap* m_rtvHeap;
	ID3D12DescriptorHeap* m_dsvHeap;
	UINT m_rtvDescriptorSize;
	ID3D12Resource * m_depthStencil;

	// Synchronization objects.
	UINT m_frameIndex;
	HANDLE m_fenceEvent;
	ID3D12Fence* m_fence;
	UINT64 M_ShadowFence = 0;
	UINT64 m_fenceValues[RHI::CPUFrameCount];

	ID3D12Debug* debugController;
	HANDLE m_VideoMemoryBudgetChange;
	DWORD m_BudgetNotificationCookie;

	typedef std::pair<IUnknown*, int64_t> UploadHeapStamped;
	std::vector<UploadHeapStamped> DeferredDeleteQueue;

	class GPUResource* m_RenderTargetResources[RHI::CPUFrameCount];
	size_t usedVRAM = 0;
	size_t totalVRAM = 0;
	class D3D12ReadBackCopyHelper* ScreenShotter = nullptr;
	bool Omce = false;
	bool RunScreenShot = false;
};
#include "D3D12Helpers.h"
//helper functions!
static inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		ensureFatalMsgf(hr == S_OK, +(std::string)D3D12Helpers::DXErrorCodeToString(hr));
	}
}


