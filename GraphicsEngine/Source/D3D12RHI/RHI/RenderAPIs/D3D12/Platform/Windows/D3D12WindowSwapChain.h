#pragma once
#if SUPPORT_DXGI
class D3D12CommandList;
class D3D12WindowSwapChain
{
public:
	D3D12WindowSwapChain();
	~D3D12WindowSwapChain();
	void Present();
	void Init(int BufferCount, int PresentInterval,glm::ivec2 Size);
	void Resize(int width, int height);
	void Release();


	void CreateSwapChainRTs();
	void ReleaseSwapRTs();
	int GetSwapChainIndex();
	void ClearRenderTarget(ID3D12GraphicsCommandList * MainList);
	void RenderToScreen(ID3D12GraphicsCommandList * list);
	void SetScreenRenderTarget(D3D12CommandList * list);
	void CreateDepthStencil(int width, int height);
	IDXGIFactory4 * factory = nullptr; 
	IDXGISwapChain3* m_swapChain = nullptr;
	ID3D12Resource* m_SwaprenderTargets[RHI::CPUFrameCount] = { nullptr,nullptr };
	GPUResource* m_RenderTargetResources[RHI::CPUFrameCount] = { 0,0 };

	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;

	ID3D12DescriptorHeap* m_rtvHeap = nullptr;
	ID3D12DescriptorHeap* m_dsvHeap = nullptr;
	UINT m_rtvDescriptorSize;

	GPUResource* m_depthStencil = nullptr;
	int m_frameIndex = 0;
	int m_width = 0;
	int m_height = 0;
};

#endif