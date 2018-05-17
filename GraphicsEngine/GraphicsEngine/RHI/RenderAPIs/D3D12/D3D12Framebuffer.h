#pragma once
#include "Rendering/Core/FrameBuffer.h"
#include "../EngineGlobals.h"
#include <d3d12.h>
#include "d3dx12.h"
#include "D3D12Shader.h"
class D3D12FrameBuffer : public FrameBuffer
{
public:
	D3D12FrameBuffer(int width, int height, class DeviceContext* device, float ratio = 1.0f, FrameBuffer::FrameBufferType type = FrameBufferType::ColourDepth) :FrameBuffer(width, height, ratio, type)
	{
		CurrentDevice = device;
	}
	virtual ~D3D12FrameBuffer();
	// Inherited via FrameBuffer
	void		 BindBufferToTexture(CommandListDef * list, int slot, int Resourceindex =0);
	virtual void BindBufferAsRenderTarget(CommandListDef * list = nullptr) override;
	void		 UnBind(CommandListDef * list);
	virtual void UnBind() override {};
	virtual void ClearBuffer(CommandListDef * list = nullptr) override;
	D3D12Shader::PipeRenderTargetDesc GetPiplineRenderDesc();
	void		 CreateCubeDepth() override;
	void		 CreateColour(int Index = 0) override;
	void		 CreateSRV();
	void		 CreateDepth() override;
	void		 CreateGBuffer() override;
	bool CheckDevice(int index);
	void Resize(int width, int height) override;
	void SetupCopyToDevice(DeviceContext* device);
	void CopyToDevice(DeviceContext* device);
	
private:
	
	class DescriptorHeap* SrvHeap = nullptr;
	class DescriptorHeap* RTVHeap = nullptr;
	class DescriptorHeap* DSVHeap = nullptr;
	class DescriptorHeap* NullHeap = nullptr;

	ID3D12Resource * m_depthStencil = nullptr;	
	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;

	DXGI_FORMAT RTVformat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT Depthformat = DXGI_FORMAT_D32_FLOAT;
	
	ID3D12DescriptorHeap* m_rtvHeap = nullptr;
	bool once = false;
	int lastboundslot = 0;
	const float CubeDepthclearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	class GPUResource* DepthStencil = nullptr;
	class GPUResource* RenderTarget[8] = {};
};

