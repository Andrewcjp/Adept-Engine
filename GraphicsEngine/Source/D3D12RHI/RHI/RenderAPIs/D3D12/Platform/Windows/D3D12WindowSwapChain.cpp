#include "D3D12WindowSwapChain.h"
#if SUPPORT_DXGI
#include "../../D3D12RHI.h"
#include "../../D3D12DeviceContext.h"
#include "../../GPUResource.h"
#include "../../D3D12CommandList.h"
#include "../../DXMemoryManager.h"


D3D12WindowSwapChain::D3D12WindowSwapChain()
{}


D3D12WindowSwapChain::~D3D12WindowSwapChain()
{}

void D3D12WindowSwapChain::Present()
{
	ThrowIfFailed(m_swapChain->Present(0, 0));
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void D3D12WindowSwapChain::Init(int BufferCount, int PresentInterval, glm::ivec2 Size)
{
	m_width = Size.x;
	m_height = Size.y;
	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = RHI::CPUFrameCount;
	swapChainDesc.Width = Size.x;
	swapChainDesc.Height = Size.y;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;
	IDXGISwapChain1* swapChain;
	ThrowIfFailed(factory->CreateSwapChainForHwnd(
		D3D12RHI::Get()->GetPrimaryDevice()->GetCommandQueue(),		// Swap chain needs the queue so that it can force a flush on it.
		PlatformWindow::GetHWND(),
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));

	m_swapChain = (IDXGISwapChain3*)swapChain;
	// This sample does not support fullscreen transitions.
	ThrowIfFailed(factory->MakeWindowAssociation(PlatformWindow::GetHWND(), DXGI_MWA_NO_ALT_ENTER));
	m_swapChain->SetFullscreenState(false, nullptr);
	SafeRelease(factory);

	m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height));
	m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height));
	// Create descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = RHI::CPUFrameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(D3D12RHI::Get()->GetDisplayDevice()->CreateDescriptorHeap(&rtvHeapDesc, ID_PASS(&m_rtvHeap)));

		m_rtvDescriptorSize = D3D12RHI::Get()->GetDisplayDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(D3D12RHI::Get()->GetDisplayDevice()->CreateDescriptorHeap(&dsvHeapDesc, ID_PASS(&m_dsvHeap)));
		m_rtvDescriptorSize = D3D12RHI::Get()->GetDisplayDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	}
	CreateDepthStencil(m_width, m_height);
	CreateSwapChainRTs();
}

void D3D12WindowSwapChain::Resize(int width, int height)
{
	ReleaseSwapRTs();
	ThrowIfFailed(m_swapChain->ResizeBuffers(RHI::CPUFrameCount, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
	m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
	m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(width), static_cast<LONG>(height));
	CreateSwapChainRTs();

	CreateDepthStencil(width, height);
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void D3D12WindowSwapChain::Release()
{
	if (m_swapChain != nullptr)
	{
		m_swapChain->SetFullscreenState(false, nullptr);
	}
	ReleaseSwapRTs();
	SafeRelease(m_swapChain);
	SafeRelease(m_rtvHeap);
	SafeRelease(m_dsvHeap);
	factory = nullptr;
}

void D3D12WindowSwapChain::CreateSwapChainRTs()
{
#if SUPPORT_DXGI
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
	// Create a RTV for each frame.
	for (UINT n = 0; n < RHI::CPUFrameCount; n++)
	{
		ThrowIfFailed(m_swapChain->GetBuffer(n, ID_PASS(&m_SwaprenderTargets[n])));
		D3D12RHI::Get()->GetDisplayDevice()->CreateRenderTargetView(m_SwaprenderTargets[n], nullptr, rtvHandle);
		m_RenderTargetResources[n] = new GPUResource(m_SwaprenderTargets[n], D3D12_RESOURCE_STATE_PRESENT, RHI::GetDefaultDevice());
		rtvHandle.Offset(1, m_rtvDescriptorSize);
	}
	NAME_D3D12_OBJECT(m_SwaprenderTargets[1]);
	NAME_D3D12_OBJECT(m_SwaprenderTargets[0]);
#endif
}

void D3D12WindowSwapChain::ReleaseSwapRTs()
{
	for (UINT n = 0; n < RHI::CPUFrameCount; n++)
	{
		SafeDelete(m_RenderTargetResources[n]);
	}
	SafeRelease(m_depthStencil);
}

int D3D12WindowSwapChain::GetSwapChainIndex()
{
	return m_frameIndex;
}

void D3D12WindowSwapChain::ClearRenderTarget(ID3D12GraphicsCommandList* MainList)
{
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	MainList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	MainList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}

void D3D12WindowSwapChain::RenderToScreen(ID3D12GraphicsCommandList* list)
{
	list->RSSetViewports(1, &m_viewport);
	list->RSSetScissorRects(1, &m_scissorRect);
}

void D3D12WindowSwapChain::SetScreenRenderTarget(D3D12CommandList* list)
{
	m_RenderTargetResources[m_frameIndex]->SetResourceState(list, D3D12_RESOURCE_STATE_RENDER_TARGET);
	list->FlushBarriers();
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	list->GetCommandList()->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
}

void D3D12WindowSwapChain::CreateDepthStencil(int width, int height)
{
	//create the depth stencil for the screen
	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	AllocDesc Desc = {};
	Desc.ResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
	Desc.InitalState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	Desc.ClearValue = depthOptimizedClearValue;
	Desc.UseCommittedResource = true;
	Desc.Name = "SwapChain depth stencil";
	D3D12RHI::Get()->GetPrimaryDevice()->GetMemoryManager()->AllocResource(Desc, &m_depthStencil);

	D3D12RHI::Get()->GetDisplayDevice()->CreateDepthStencilView(m_depthStencil->GetResource(), &depthStencilDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
}

#endif