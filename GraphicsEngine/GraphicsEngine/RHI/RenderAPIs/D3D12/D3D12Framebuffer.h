#pragma once
#include "Rendering/Core/FrameBuffer.h"
#include "EngineGlobals.h"
#include <d3d12.h>
#include "d3dx12.h"
#include "D3D12Shader.h"
class DescriptorHeap;
class GPUResource;
class D3D12FrameBuffer : public FrameBuffer
{
public:
	D3D12FrameBuffer(class DeviceContext* device, RHIFrameBufferDesc& Desc);
	void UpdateSRV();
	void CreateResource(GPUResource ** Resourceptr, DescriptorHeap * Heapptr, bool IsDepthStencil, DXGI_FORMAT Format, eTextureDimension ViewDimension, int OffsetInHeap = 0);
	void Init();
	virtual ~D3D12FrameBuffer();
	void ReadyResourcesForRead(CommandListDef * list, int Resourceindex = 0);
	// Inherited via FrameBuffer
	void		 BindBufferToTexture(CommandListDef * list, int slot, int Resourceindex =0, DeviceContext* target = nullptr);
	virtual void BindBufferAsRenderTarget(CommandListDef * list = nullptr) ;
	void		 UnBind(CommandListDef * list);
	virtual void ClearBuffer(CommandListDef * list = nullptr) ;
	D3D12Shader::PipeRenderTargetDesc GetPiplineRenderDesc();
	void			CreateSRVHeap(int Num);
	void CreateSRVInHeap(int HeapOffset, DescriptorHeap * targetheap);
	void CreateSRVInHeap(int HeapOffset, DescriptorHeap * targetheap, DeviceContext * target);
	D3D12_SHADER_RESOURCE_VIEW_DESC GetSrvDesc(int RenderTargetIndex);
	bool CheckDevice(int index);
	void Resize(int width, int height) override;

	//Cross Adaptor
	void SetupCopyToDevice(DeviceContext* device) override;
	void TransitionTOCopy(ID3D12GraphicsCommandList * list);
	void CopyToDevice(ID3D12GraphicsCommandList * list);
	void MakeReadyOnTarget(ID3D12GraphicsCommandList * list);
	void MakeReadyForCopy(ID3D12GraphicsCommandList * list);

	void BindDepthWithColourPassthrough(ID3D12GraphicsCommandList* list,D3D12FrameBuffer* Passtrhough);
private:
	void MakeReadyForRead(ID3D12GraphicsCommandList * list);
	DescriptorHeap* SrvHeap = nullptr;
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

