#pragma once
#include <d3d12.h>
#include <DXGI1_4.h>
#include "d3dx12.h"
#include <DirectXMath.h>
//this class is to wrap all d3d12 rhi stuff up in
//and handle object creation
//and prevent RHI.cpp from getting extremly large
#include <vector>
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
//d3d12.lib;dxgi.lib;d3dcompiler.lib;
#include "d3d12Shader.h"
#include "D3D12Mesh.h"
#include "D3D12Texture.h"

class D3D12RHI
{
public:


	static D3D12RHI* Instance;
	D3D12RHI();
	~D3D12RHI();
	void InitContext();
	void DestroyContext();
	void PresentFrame();
	void OnDestroy();
	void ClearRenderTarget(ID3D12GraphicsCommandList * MainList);
	void RenderToScreen(ID3D12GraphicsCommandList * list);
	void PreFrameSetUp(ID3D12GraphicsCommandList * list, D3D12Shader * Shader);
	void PreFrameSwap(ID3D12GraphicsCommandList* list);
	void LoadPipeLine();
	void LoadAssets();
	void ExecSetUpList();
	void InitliseDefaults();
	void ExecList(CommandListDef * list);
	/*struct SceneConstantBuffer
	{
		DirectX::XMFLOAT4 offset;
	};*/
	int TextureWidth = 100;
	int TextureHeight = 100;
	static const UINT TexturePixelSize = 4;	// The number of bytes used to represent a pixel in the texture.
	D3D12Shader* testshader;
	D3D12Mesh* testmesh;
	void PopulateCommandList();
	void PostFrame(ID3D12GraphicsCommandList * list);
	void WaitForPreviousFrame();
	void WaitForGpu();
	void MoveToNextFrame();
	//ptr
	static const int FrameCount = 2;
	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;
	IDXGISwapChain3* m_swapChain;
	ID3D12Device* m_device;
	ID3D12Resource* m_renderTargets[FrameCount];
	ID3D12CommandAllocator* m_commandAllocator;
	ID3D12CommandQueue* m_commandQueue;
	ID3D12RootSignature* m_rootSignature;
	ID3D12DescriptorHeap* m_rtvHeap;
	ID3D12DescriptorHeap* m_dsvHeap;
	//ID3D12PipelineState* m_pipelineState;
	ID3D12GraphicsCommandList* m_commandList;
	ID3D12GraphicsCommandList* m_SetupCommandList;
	UINT m_rtvDescriptorSize;
	ID3D12Resource * m_depthStencil;

		
	ID3D12Resource* m_constantBuffer;
	D3D12Shader::SceneConstantBuffer m_constantBufferData;
	UINT8* m_pCbvDataBegin;
	ID3D12DescriptorHeap* m_cbvHeap;

	D3D12Texture* texture;
	D3D12Texture* OtherTex;
	//ID3D12DescriptorHeap* m_srvHeap;
	//ID3D12Resource* m_texture;
	/*ID3D12Resource* m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
*/
// Synchronization objects.
	UINT m_frameIndex;
	HANDLE m_fenceEvent;
	ID3D12Fence* m_fence;
	UINT64 m_fenceValue;
	UINT64 m_fenceValues[FrameCount];
	D3D12Shader::PiplineShader m_MainShaderPiplineShader;

	int m_width = 100;
	int m_height = 100;
	float m_aspectRatio = 0.0f;
	static ID3D12Device* GetDevice()
	{
		if (Instance != nullptr)
		{
			return Instance->m_device;
		}
		return nullptr;
	}
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
