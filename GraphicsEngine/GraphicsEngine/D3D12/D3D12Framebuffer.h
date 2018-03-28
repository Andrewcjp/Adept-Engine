#pragma once
#include "Rendering/Core/FrameBuffer.h"
#include "../EngineGlobals.h"
#include <d3d12.h>
#include "d3dx12.h"
class D3D12FrameBuffer
	:public FrameBuffer
{
public:
	D3D12FrameBuffer(int width, int height, float ratio = 1.0f, FrameBuffer::FrameBufferType type = FrameBufferType::ColourDepth) :FrameBuffer(width, height, ratio, type) {
		CreateBuffer();
	}
	void CreateBuffer();
	void CreateCubeDepth();
	void CreateDepth();
	~D3D12FrameBuffer();
	// Inherited via FrameBuffer
	virtual void BindToTextureUnit(int unit = 0) override;
	void BindBufferToTexture(CommandListDef * list,int slot);
	virtual void BindBufferAsRenderTarget(CommandListDef * list = nullptr) override;

	void UnBind(CommandListDef * list);

	virtual void UnBind() override {};

	virtual void ClearBuffer(CommandListDef * list = nullptr) override;
private:
	ID3D12DescriptorHeap* m_dsvHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthHandle;
	ID3D12DescriptorHeap* m_srvHeap;
	ID3D12DescriptorHeap* m_nullHeap;
	ID3D12Resource * m_depthStencil;
	CD3DX12_GPU_DESCRIPTOR_HANDLE NullHandle;
	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;
	bool once = false;
	int lastboundslot = 0;
};

