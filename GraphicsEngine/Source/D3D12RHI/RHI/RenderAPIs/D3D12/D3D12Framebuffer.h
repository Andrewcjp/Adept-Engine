#pragma once
#include "Rendering/Core/FrameBuffer.h"
#include "Core/Utils/RefChecker.h"
class DescriptorHeap;
class GPUResource;
class D3D12DeviceContext;
class DXDescriptor;
class D3D12CommandList;
class GPUMemoryPage;
class D3D12RHITexture;
class D3D12FrameBuffer : public FrameBuffer
{
public:
	D3D12FrameBuffer(class DeviceContext* device, const RHIFrameBufferDesc& Desc);
	virtual ~D3D12FrameBuffer();

	void Init();

	void CreateHeaps();

	void CreateFromTextures();

	void							CreateRTDescriptor(D3D12RHITexture * Texture, DescriptorHeap * Heapptr, int OffsetInHeap);

	// Inherited via FrameBuffer
	void							BindBufferToTexture(ID3D12GraphicsCommandList * list, int slot, int Resourceindex = 0, DeviceContext* target = nullptr, bool isCompute = false);
	virtual void					BindBufferAsRenderTarget(D3D12CommandList * list, int SubResourceIndex);
	void							UnBind(ID3D12GraphicsCommandList * list);
	virtual void					ClearBuffer(D3D12CommandList * list = nullptr);
	D3D12_SHADER_RESOURCE_VIEW_DESC GetSrvDesc(int RenderTargetIndex);
	static D3D12_SHADER_RESOURCE_VIEW_DESC GetSrvDesc(int RenderTargetIndex, const RHIFrameBufferDesc & desc);
	bool							CheckDevice(int index);
	void							HandleResize() override;
	void ValidateState();
	bool							IsReadyForCompute()const;
	virtual const RHIPipeRenderTargetDesc& GetPiplineRenderDesc();


	virtual void BindDepthWithColourPassthrough(class RHICommandList* list, FrameBuffer* PassThrough) override;
	DeviceContext* GetDevice() override;
	GPUResource* GetResource(int index) const;
	void Release() override;
	virtual void CopyToOtherBuffer(FrameBuffer * OtherBuffer, RHICommandList* List) override;
	virtual void SetResourceState(RHICommandList* List, EResourceState::Type State, bool ChangeDepth = false, EResourceTransitionMode::Type TransitionMode = EResourceTransitionMode::Direct) override;
	DXDescriptor * GetDescriptor(const RHIViewDesc & desc, DescriptorHeap * heap = nullptr);

	virtual uint64 GetInstanceHash() const override;
	static	D3D12_RESOURCE_STATES ConvertState(EResourceState::Type State);
	void PopulateDescriptor(DXDescriptor* desc, int index, const RHIViewDesc& view);
	uint64 GetViewHash(const RHIViewDesc& desc);
private:
	void SetState(RHICommandList* List, D3D12_RESOURCE_STATES state, bool depth, EResourceTransitionMode::Type TransitionMode);
	D3D12DeviceContext * CurrentDevice = nullptr;

	DescriptorHeap* RTVHeap = nullptr;
	DescriptorHeap* DSVHeap = nullptr;
	DescriptorHeap* NullHeap = nullptr;
	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_VIEWPORT m_viewports[8];
	CD3DX12_RECT m_scissorRect;
	CD3DX12_RECT m_scissorRects[8];
	DXGI_FORMAT RTVformat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT Depthformat = DXGI_FORMAT_D32_FLOAT;
	DXGI_FORMAT DepthReadformat = DXGI_FORMAT_R32_FLOAT;
	const DXGI_FORMAT DefaultDepthformat = DXGI_FORMAT_D32_FLOAT;
	const DXGI_FORMAT DefaultDepthReadformat = DXGI_FORMAT_R32_FLOAT;
	ID3D12DescriptorHeap* m_rtvHeap = nullptr;
	bool once = false;
	int lastboundslot = 0;
	D3D12RHITexture* DepthStencil = nullptr;
	D3D12RHITexture* RenderTarget[8] = {};

	//device Sharing
	CD3DX12_RESOURCE_DESC renderTargetDesc;
	RHIPipeRenderTargetDesc RenderTargetDesc;

};

CreateChecker(D3D12FrameBuffer);