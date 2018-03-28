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
#include "d3d12Shader.h"
#include "D3D12Mesh.h"
#include "D3D12Texture.h"
#if defined(_DEBUG)
inline void SetName(ID3D12Object* pObject, LPCWSTR name)
{
	pObject->SetName(name);
}
#else
inline void SetName(ID3D12Object*, LPCWSTR)
{}
#endif
#define NAME_D3D12_OBJECT(x) SetName(x, L#x)
#define USEGPUTOGENMIPS 1
class D3D12RHI
{
public:
	static D3D12RHI* Instance;
	D3D12RHI();
	~D3D12RHI();
	void InitContext();
	void DestroyContext();
	void PresentFrame();
	void ClearRenderTarget(ID3D12GraphicsCommandList * MainList);
	void RenderToScreen(ID3D12GraphicsCommandList * list);
	static void PreFrameSetUp(ID3D12GraphicsCommandList * list, D3D12Shader * Shader);
	void PreFrameSwap(ID3D12GraphicsCommandList* list);
	void SetScreenRenderTaget(ID3D12GraphicsCommandList * list);
	D3D_FEATURE_LEVEL GetMaxSupportedFeatureLevel(ID3D12Device * pDevice);
	void DisplayDeviceDebug();
	void LoadPipeLine();
	void CreateSwapChainRTs();
	void InitMipmaps();
	void InternalResizeSwapChain(int x, int y);
	void ReleaseSwapRTs();
	void ResizeSwapChain(int x, int y);
	void CreateDepthStencil(int width, int height);
	void LoadAssets();
	void ExecSetUpList();
	void ExecList(CommandListDef * list, bool IsFinal = false);
	void TransitionBuffers(bool In);
	void PostFrame(ID3D12GraphicsCommandList * list);
	void WaitForPreviousFrame();
	void WaitForGpu();
	void MoveToNextFrame();
	static ID3D12Device* GetDevice();
	ID3D12GraphicsCommandList* m_SetupCommandList;
	int m_width = 100;
	int m_height = 100;
	float m_aspectRatio = 0.0f;
	bool HasSetup = false;
	class ShaderMipMap* MipmapShader = nullptr;
	ID3D12DescriptorHeap* BaseTextureHeap;
private:
	static const int FrameCount = 2;
	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;
	IDXGISwapChain3* m_swapChain;
	ID3D12Device* m_Primarydevice;
	ID3D12Device* m_Secondarydevice;
	ID3D12Resource* m_renderTargets[FrameCount];
	ID3D12CommandAllocator* m_commandAllocator;
	ID3D12CommandQueue* m_commandQueue;
	ID3D12DescriptorHeap* m_rtvHeap;
	ID3D12DescriptorHeap* m_dsvHeap;
	UINT m_rtvDescriptorSize;
	ID3D12Resource * m_depthStencil;
	bool RequestedResize = false;
	int newwidth = 0;
	int newheight = 0;
	
	IDXGIAdapter3* pDXGIAdapter = nullptr;
	// Synchronization objects.
	UINT m_frameIndex;
	HANDLE m_fenceEvent;
	ID3D12Fence* m_fence;
	UINT64 m_fenceValue;
	UINT64 M_ShadowFence = 0;
	ID3D12Fence* pShadowFence;
	UINT64 m_fenceValues[FrameCount];
	D3D12Shader::PiplineShader m_MainShaderPiplineShader;
	//	HANDLE ShadowExechandle;
	ID3D12Debug* debugController;
	HANDLE m_VideoMemoryBudgetChange;
	DWORD m_BudgetNotificationCookie;
	int count = 0;
	
	
};
//helper functions!
static inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		__debugbreak();
		throw std::exception();
	}
}
void GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter);
