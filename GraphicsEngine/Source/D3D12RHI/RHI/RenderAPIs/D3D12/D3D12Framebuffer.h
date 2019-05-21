#pragma once
#include "Rendering/Core/FrameBuffer.h"
#include "Core/Utils/RefChecker.h"
class DescriptorHeap;
class GPUResource;
class D3D12DeviceContext;
class Descriptor;
class D3D12FrameBuffer : public FrameBuffer
{
public:
	D3D12FrameBuffer(class DeviceContext* device, const RHIFrameBufferDesc& Desc);
	virtual ~D3D12FrameBuffer();

	void UpdateSRV();
	void Init();

	void							CreateResource(GPUResource ** Resourceptr, DescriptorHeap * Heapptr, bool IsDepthStencil, DXGI_FORMAT Format, eTextureDimension ViewDimension, int OffsetInHeap = 0);
	void							ReadyResourcesForRead(ID3D12GraphicsCommandList * list, int Resourceindex = 0);
	// Inherited via FrameBuffer
	void							BindBufferToTexture(ID3D12GraphicsCommandList * list, int slot, int Resourceindex = 0, DeviceContext* target = nullptr, bool isCompute = false);
	virtual void					BindBufferAsRenderTarget(ID3D12GraphicsCommandList * list, int SubResourceIndex);
	void							UnBind(ID3D12GraphicsCommandList * list);
	virtual void					ClearBuffer(ID3D12GraphicsCommandList * list = nullptr);
	void							CreateSRVHeap(int Num);
	void							CreateSRVInHeap(int HeapOffset, Descriptor* desc);
	void							CreateSRVInHeap(int HeapOffset, Descriptor* desc, DeviceContext * target);
	void CreateDepthSRV(int HeapOffset, Descriptor * desc);
	D3D12_SHADER_RESOURCE_VIEW_DESC GetSrvDesc(int RenderTargetIndex);
	static D3D12_SHADER_RESOURCE_VIEW_DESC GetSrvDesc(int RenderTargetIndex, RHIFrameBufferDesc & desc);
	bool							CheckDevice(int index);
	void							HandleResize() override;
	bool							IsReadyForCompute()const;
	virtual const RHIPipeRenderTargetDesc& GetPiplineRenderDesc();
	//Cross Adaptor
	void SetupCopyToDevice(DeviceContext* device) override;
	void TransitionTOCopy(ID3D12GraphicsCommandList * list);
	void CopyToHostMemory(ID3D12GraphicsCommandList * list);
	void CopyFromHostMemory(ID3D12GraphicsCommandList * list);
	void MakeReadyForCopy_In(ID3D12GraphicsCommandList * list);

	virtual void MakeReadyForComputeUse(RHICommandList* List, bool Depth = false) override;
	virtual void BindDepthWithColourPassthrough(class RHICommandList* list, FrameBuffer* PassThrough) override;
	DeviceContext* GetTargetDevice();
	DeviceContext* GetDevice() override;
	GPUResource* GetResource(int index);
	void Release() override;


	virtual void CopyToOtherBuffer(FrameBuffer * OtherBuffer, RHICommandList* List) override;




private:
	D3D12DeviceContext * CurrentDevice = nullptr;
	void MakeReadyForRead(ID3D12GraphicsCommandList * list);
	void MakeReadyForCopy(RHICommandList * list) override;
	//DescriptorHeap* SrvHeap = nullptr;
	Descriptor* SRVDesc = nullptr;
	DescriptorHeap* RTVHeap = nullptr;
	DescriptorHeap* DSVHeap = nullptr;
	DescriptorHeap* NullHeap = nullptr;

	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;

	DXGI_FORMAT RTVformat = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_FORMAT Depthformat = DXGI_FORMAT_D32_FLOAT;
	DXGI_FORMAT DepthReadformat = DXGI_FORMAT_R32_FLOAT;
	const DXGI_FORMAT DefaultDepthformat = DXGI_FORMAT_D32_FLOAT;
	const DXGI_FORMAT DefaultDepthReadformat = DXGI_FORMAT_R32_FLOAT;
	ID3D12DescriptorHeap* m_rtvHeap = nullptr;
	bool once = false;
	int lastboundslot = 0;
	class GPUResource* DepthStencil = nullptr;
	class GPUResource* RenderTarget[8] = {};

	//device Sharing
	class D3D12DeviceContext* OtherDevice = nullptr;
	ID3D12Heap* CrossHeap = nullptr;
	ID3D12Heap* TWO_CrossHeap = nullptr;
	HANDLE heapHandle = nullptr;
	ID3D12Resource* PrimaryRes = nullptr;
	ID3D12Resource* Stagedres = nullptr;
	ID3D12Resource* FinalOut = nullptr;
	CD3DX12_RESOURCE_DESC renderTargetDesc;
	Descriptor* SharedSRVHeap = nullptr;
	class GPUResource* TargetCopy = nullptr;
	RHIPipeRenderTargetDesc RenderTargetDesc = {};

};

CreateChecker(D3D12FrameBuffer);