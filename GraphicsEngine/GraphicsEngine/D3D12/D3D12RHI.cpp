#include "stdafx.h"
#include "D3D12RHI.h"
#include "../BaseApplication.h"
#include <D3Dcompiler.h>
#include "glm\glm.hpp"
#include "include\glm\gtx\transform.hpp"
D3D12RHI* D3D12RHI::Instance = nullptr;
D3D12RHI::D3D12RHI()
{
	Instance = this;
}


D3D12RHI::~D3D12RHI()
{
}

void D3D12RHI::InitContext()
{
}

void D3D12RHI::DestroyContext()
{
}
void EnableShaderBasedValidation()
{
	ID3D12Debug* spDebugController0;
	ID3D12Debug1* spDebugController1;
	(D3D12GetDebugInterface(IID_PPV_ARGS(&spDebugController0)));
	(spDebugController0->QueryInterface(IID_PPV_ARGS(&spDebugController1)));
	spDebugController1->SetEnableGPUBasedValidation(true);
}
void D3D12RHI::LoadPipeLine()
{
	//	EnableShaderBasedValidation();
	m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(m_width), static_cast<float>(m_height));
	m_scissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height));
	UINT dxgiFactoryFlags = 0;
#if defined(_DEBUG)
	// Enable the debug layer (requires the Graphics Tools "optional feature").
	// NOTE: Enabling the debug layer after device creation will invalidate the active device.
	{
		ID3D12Debug* debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();

			// Enable additional debug layers.
			dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}
#endif
	IDXGIFactory4* factory;
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

	if (false/*m_useWarpDevice*/)
	{
		IDXGIAdapter* warpAdapter;
		ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(
			warpAdapter,
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_device)
		));
	}
	else
	{
		IDXGIAdapter1* hardwareAdapter;
		GetHardwareAdapter(factory, &hardwareAdapter);

		ThrowIfFailed(D3D12CreateDevice(
			hardwareAdapter,
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_device)
		));
	}

	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

	// Describe and create the swap chain.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.Width = m_width;
	swapChainDesc.Height = m_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	IDXGISwapChain1* swapChain;
	ThrowIfFailed(factory->CreateSwapChainForHwnd(
		m_commandQueue,		// Swap chain needs the queue so that it can force a flush on it.
		BaseApplication::GetHWND(),
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain
	));
	m_swapChain = (IDXGISwapChain3*)swapChain;
	// This sample does not support fullscreen transitions.
	ThrowIfFailed(factory->MakeWindowAssociation(BaseApplication::GetHWND(), DXGI_MWA_NO_ALT_ENTER));

	//ThrowIfFailed(swapChain.As(&m_swapChain));
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	// Create descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = FrameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

		m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
		m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		//assert(result == S_OK && "ERROR CREATING THE DSV HEAP");
		//CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
		//	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	}


	// Create frame resources.
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

		// Create a RTV for each frame.
		for (UINT n = 0; n < FrameCount; n++)
		{
			ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
			m_device->CreateRenderTargetView(m_renderTargets[n], nullptr, rtvHandle);
			rtvHandle.Offset(1, m_rtvDescriptorSize);
		}
	}

	ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
}

void D3D12RHI::LoadAssets()
{

	
	ID3DBlob* vertexShader;
	ID3DBlob* pixelShader;
	// Create the pipeline state, which includes compiling and loading shaders.
	{


#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif
		HRESULT hr = S_OK;
		ID3DBlob* pErrorBlob = NULL;
		hr = D3DCompileFromFile((L"shaders.hlsl"), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, &pErrorBlob);

		hr = D3DCompileFromFile((L"shaders.hlsl"), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, &pErrorBlob);

		if (pErrorBlob)
		{
			fprintf(stdout, "Shader output: %s\n",
				reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));

			pErrorBlob->Release();
		}
		if (FAILED(hr))
		{
			__debugbreak();
		}
		///using combind complie so cant use this
	}

	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};


	testmesh = new D3D12Mesh();
	testshader = new D3D12Shader();
	testshader->m_vsBlob = vertexShader;
	testshader->m_fsBlob = pixelShader;

	testshader->m_Shader = testshader->CreatePipelineShader(inputElementDescs, _countof(inputElementDescs), vertexShader, pixelShader);
	testshader->InitCBV();
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator, testshader->m_Shader.m_pipelineState, IID_PPV_ARGS(&m_SetupCommandList)));
	texture = new D3D12Texture();

	{
		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

		D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
		depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
		depthOptimizedClearValue.DepthStencil.Stencil = 0;

		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, m_width, m_height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthOptimizedClearValue,
			IID_PPV_ARGS(&m_depthStencil)
		));

		//NAME_D3D12_OBJECT(m_depthStencil);

		m_device->CreateDepthStencilView(m_depthStencil, &depthStencilDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	}
	//	m_constantBufferData.M = glm::translate(glm::vec3(-10,0,-10));
	m_constantBufferData.P = glm::perspectiveLH(glm::radians(70.0f), 1.77f, 0.1f, 1000.0f);
	glm::vec3 eye = glm::vec3(0.0f, 10.0f, -10.0f);
	glm::vec3 at = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	//m_view = glm::lookAtLH(Lights[0]->GetPosition(), at, up);
	m_constantBufferData.V = glm::lookAtLH(eye, at, up);

	//exec setup command list
	ThrowIfFailed(m_SetupCommandList->Close());
	ID3D12CommandList* ppCommandLists[] = { m_SetupCommandList };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	// Create synchronization objects and wait until assets have been uploaded to the GPU.
	{
		ThrowIfFailed(m_device->CreateFence(m_fenceValues[m_frameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
		m_fenceValues[m_frameIndex]++;
		//	m_fenceValue = 1;

			// Create an event handle to use for frame synchronization.
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_fenceEvent == nullptr)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}

		// Wait for the command list to execute; we are reusing the same command 
		// list in our main loop but for now, we just want to wait for setup to 
		// complete before continuing.
		WaitForGpu();
	}
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator, testshader->m_Shader.m_pipelineState, IID_PPV_ARGS(&m_commandList)));
	ThrowIfFailed(m_commandList->Close());
}
void D3D12RHI::InitliseDefaults()
{

}
void D3D12RHI::OnRender()
{
	const float translationSpeed = 0.001f;
	const float offsetBounds = 1.25f;

	//m_constantBufferData.offset.x += translationSpeed;
	//if (m_constantBufferData.offset.x > offsetBounds)
	//{
	//	m_constantBufferData.offset.x = -offsetBounds;
	//}
	////	memcpy(m_pCbvDataBegin, &m_constantBufferData, sizeof(m_constantBufferData));

	//testshader->UpdateCBV(m_constantBufferData);

	// Record all the commands we need to render the scene into the command list.
	PopulateCommandList();

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { m_commandList };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Present the frame.
	ThrowIfFailed(m_swapChain->Present(1, 0));

	MoveToNextFrame();
	WaitForGpu();
}
void D3D12RHI::OnDestroy()
{
	// Ensure that the GPU is no longer referencing resources that are about to be
	// cleaned up by the destructor.
	WaitForGpu();

	CloseHandle(m_fenceEvent);
}
void D3D12RHI::ClearRenderTarget(ID3D12GraphicsCommandList* List)
{
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	List->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	List->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);


}
void D3D12RHI::RenderToScreen(ID3D12GraphicsCommandList* list)
{
	list->RSSetViewports(1, &m_viewport);
	list->RSSetScissorRects(1, &m_scissorRect);
}

void D3D12RHI::PopulateCommandList()
{
	// Command list allocators can only be reset when the associated 
	// command lists have finished execution on the GPU; apps should use 
	// fences to determine GPU execution progress.
	ThrowIfFailed(m_commandAllocator->Reset());

	// However, when ExecuteCommandList() is called on a particular command 
	// list, that command list can then be reset at any time and must be before 
	// re-recording.
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator, testshader->m_Shader.m_pipelineState));

	//CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	//m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0, 0, 0, nullptr);

	// Set necessary state.
	m_commandList->SetGraphicsRootSignature(testshader->m_Shader.m_rootSignature);



	RenderToScreen(m_commandList);
	// Indicate that the back buffer will be used as a render target.
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	ClearRenderTarget(m_commandList);
	// Record commands.
	texture->Bind(m_commandList);

	m_constantBufferData.M = glm::translate(glm::vec3(0, 0, 0));
	testshader->UpdateCBV(m_constantBufferData);
	testshader->PushCBVToGPU(m_commandList, 0);
	testmesh->Render(m_commandList);

	texture->Bind(m_commandList);
	m_constantBufferData.M = glm::translate(glm::vec3(0, 5, 0));
	testshader->UpdateCBV(m_constantBufferData, 1);
	testshader->PushCBVToGPU(m_commandList,1);
	testmesh->Render(m_commandList);
	// Indicate that the back buffer will now be used to present.
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	ThrowIfFailed(m_commandList->Close());
}

void D3D12RHI::WaitForPreviousFrame()
{
	// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
	// This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
	// sample illustrates how to use fences for efficient resource usage and to
	// maximize GPU utilization.

	// Signal and increment the fence value.
	const UINT64 fence = m_fenceValue;
	ThrowIfFailed(m_commandQueue->Signal(m_fence, fence));
	m_fenceValue++;

	// Wait until the previous frame is finished.
	if (m_fence->GetCompletedValue() < fence)
	{
		ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}
void D3D12RHI::WaitForGpu()
{
	// Schedule a Signal command in the queue.
	ThrowIfFailed(m_commandQueue->Signal(m_fence, m_fenceValues[m_frameIndex]));

	// Wait until the fence has been processed.
	ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
	WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

	// Increment the fence value for the current frame.
	m_fenceValues[m_frameIndex]++;
}

// Prepare to render the next frame.
void D3D12RHI::MoveToNextFrame()
{
	// Schedule a Signal command in the queue.
	const UINT64 currentFenceValue = m_fenceValues[m_frameIndex];
	ThrowIfFailed(m_commandQueue->Signal(m_fence, currentFenceValue));

	// Update the frame index.
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	// If the next frame is not ready to be rendered yet, wait until it is ready.
	if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
	{
		ThrowIfFailed(m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent));
		WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
	}

	// Set the fence value for the next frame.
	m_fenceValues[m_frameIndex] = currentFenceValue + 1;
}




void GetHardwareAdapter(IDXGIFactory2 * pFactory, IDXGIAdapter1 ** ppAdapter)
{
	IDXGIAdapter1* adapter;
	*ppAdapter = nullptr;

	for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// Don't select the Basic Render Driver adapter.
			// If you want a software adapter, pass in "/warp" on the command line.
			continue;
		}

		// Check to see if the adapter supports Direct3D 12, but don't create the
		// actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
	}

	*ppAdapter = adapter;
}
