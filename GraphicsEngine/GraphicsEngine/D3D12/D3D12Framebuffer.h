#pragma once
#include "Rendering/Core/FrameBuffer.h"
#include "../EngineGlobals.h"
#include <d3d12.h>
#include "d3dx12.h"
#include "D3D12Shader.h"
class D3D12FrameBuffer
	:public FrameBuffer
{
public:
	D3D12FrameBuffer(int width, int height, float ratio = 1.0f, FrameBuffer::FrameBufferType type = FrameBufferType::ColourDepth) :FrameBuffer(width, height, ratio, type) {}

	
	virtual ~D3D12FrameBuffer();
	// Inherited via FrameBuffer
	virtual void BindToTextureUnit(int unit = 0) override;
	void         BindBufferToTexture(CommandListDef * list, int slot);
	virtual void BindBufferAsRenderTarget(CommandListDef * list = nullptr) override;
	void		 UnBind(CommandListDef * list);
	virtual void UnBind() override {};
	virtual void ClearBuffer(CommandListDef * list = nullptr) override;
	D3D12Shader::PipeRenderTargetDesc GetPiplineRenderDesc();
	void		 CreateCubeDepth() override;
	void		 CreateColour() override;
	void		 CreateSRV();
	void		 CreateDepth() override;
private:
	D3D12_CPU_DESCRIPTOR_HANDLE DepthHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE RTHandle;
	ID3D12DescriptorHeap* m_dsvHeap = nullptr;
	ID3D12DescriptorHeap* m_srvHeap = nullptr;
	ID3D12DescriptorHeap* m_nullHeap = nullptr;
	ID3D12Resource * m_depthStencil = nullptr;
	CD3DX12_GPU_DESCRIPTOR_HANDLE NullHandle;
	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;
	DXGI_FORMAT RTVformat;
	DXGI_FORMAT Depthformat;
	ID3D12Resource * m_RenderTarget = nullptr;
	ID3D12DescriptorHeap* m_rtvHeap = nullptr;
	bool once = false;
	int lastboundslot = 0;
	const float clearColor[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
	const float CubeDepthclearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	UINT descriptorSize = 0;

	class GPUResource* DepthStencil = nullptr;
	class GPUResource* RenderTarget = nullptr;

	//ID3D12Resource* CubemapRTs[6];
};

