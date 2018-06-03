#pragma once
#include "Rendering/Core/FrameBuffer.h"
#include "../EngineGlobals.h"
#include <d3d12.h>
#include "d3dx12.h"
#include "D3D12Shader.h"
class DescriptorHeap;
class D3D12FrameBuffer : public FrameBuffer
{
public:
	D3D12FrameBuffer(int width, int height, class DeviceContext* device, float ratio = 1.0f, FrameBuffer::FrameBufferType type = FrameBufferType::ColourDepth) :FrameBuffer(width, height, ratio, type)
	{
		CurrentDevice = device;
	}
	virtual ~D3D12FrameBuffer();
	void ReadyResourcesForRead(CommandListDef * list, int Resourceindex = 0);
	// Inherited via FrameBuffer
	void		 BindBufferToTexture(CommandListDef * list, int slot, int Resourceindex =0, DeviceContext* target = nullptr);
	virtual void BindBufferAsRenderTarget(CommandListDef * list = nullptr) override;
	void		 UnBind(CommandListDef * list);
	virtual void UnBind() override {};
	virtual void ClearBuffer(CommandListDef * list = nullptr) override;
	D3D12Shader::PipeRenderTargetDesc GetPiplineRenderDesc();
	void		 CreateCubeDepth() override;
	void CreateSRVHeap(int Num);
	void		 CreateColour(int Index = 0) override;
	void CreateSRV();
	void		 CreateDepth() override;
	void		 CreateGBuffer() override;
	bool CheckDevice(int index);
	void Resize(int width, int height) override;
	void SetupCopyToDevice(DeviceContext* device);
	void CopyToDevice(ID3D12GraphicsCommandList * list);
	void MakeReadyOnTarget(ID3D12GraphicsCommandList * list);
	void BindDepthWithColourPassthrough(ID3D12GraphicsCommandList* list,D3D12FrameBuffer* Passtrhough);
	void CreateSRVInHeap(int index, DescriptorHeap * targetheap);
	DescriptorHeap* GetHeap() { return SrvHeap; };
	D3D12_SHADER_RESOURCE_VIEW_DESC GetSrvDesc();
private:
	
	DescriptorHeap* SrvHeap = nullptr;
	DescriptorHeap* RTVHeap = nullptr;
	DescriptorHeap* DSVHeap = nullptr;
	DescriptorHeap* NullHeap = nullptr;

	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;
	
	DXGI_FORMAT RTVformat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT Depthformat = DXGI_FORMAT_D32_FLOAT;
	const DXGI_FORMAT DefaultDepthformat = DXGI_FORMAT_D32_FLOAT;
	const DXGI_FORMAT DefaultDepthReadformat = DXGI_FORMAT_R32_FLOAT;
	ID3D12DescriptorHeap* m_rtvHeap = nullptr;
	bool once = false;
	int lastboundslot = 0;
	const float CubeDepthclearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f };
	class GPUResource* DepthStencil = nullptr;
	class GPUResource* RenderTarget[8] = {};
	int DSVCount = 1;

	//device Shareing
	DeviceContext* OtherDevice = nullptr;
	ID3D12Heap* CrossHeap = nullptr;
	ID3D12Heap* TWO_CrossHeap = nullptr;
	HANDLE heapHandle = nullptr;
	ID3D12Resource* PrimaryRes = nullptr;
	ID3D12Resource* Stagedres = nullptr;
	ID3D12Resource* FinalOut = nullptr;
	CD3DX12_RESOURCE_DESC renderTargetDesc;
	DescriptorHeap* SharedSRVHeap = nullptr;
	class GPUResource* SharedTarget = nullptr;
};

