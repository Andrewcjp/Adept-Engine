#pragma once
#include "RHI/RHICommandList.h"
#include <d3d12.h>
#include "D3D12Shader.h"

class D3D12CommandList : public RHICommandList
{
public:
	D3D12CommandList(DeviceContext * inDevice, ECommandListType::Type ListType = ECommandListType::Graphics);
	virtual ~D3D12CommandList();

	// Inherited via RHICommandList
	virtual void ResetList() override;
	virtual void SetRenderTarget(FrameBuffer * target, int SubResourceIndex = 0) override;
	virtual void DrawPrimitive(int VertexCountPerInstance, int InstanceCount, int StartVertexLocation, int StartInstanceLocation) override;
	virtual void DrawIndexedPrimitive(int IndexCountPerInstance, int InstanceCount, int StartIndexLocation, int BaseVertexLocation, int StartInstanceLocation) override;
	virtual void SetViewport(int MinX, int MinY, int MaxX, int MaxY, float MaxZ, float MinZ) override;
	virtual void Execute(DeviceContextQueue::Type Target) override;
	virtual void WaitForCompletion() override;
	virtual void SetVertexBuffer(RHIBuffer * buffer) override;
	virtual void SetIndexBuffer(RHIBuffer* buffer) override;
	virtual void SetPipelineState(PipeLineState state) override;
	virtual void CreatePipelineState(class Shader * shader, class FrameBuffer* Buffer = nullptr) override;
	void		 CreatePipelineState(Shader * shader, D3D12Shader::PipeRenderTargetDesc RTdesc);

	virtual void UpdateConstantBuffer(void * data, int offset) override;
	virtual void SetConstantBufferView(RHIBuffer * buffer, int offset, int Register) override;
	virtual void SetTexture(class BaseTexture * texture, int slot) override;
	virtual void SetFrameBufferTexture(FrameBuffer * buffer, int slot, int Resourceindex = 0) override;

	virtual void SetScreenBackBufferAsRT() override;
	virtual void ClearScreen() override;
	virtual void ClearFrameBuffer(FrameBuffer * buffer) override;
	virtual void UAVBarrier(class RHIUAV* target) override;
	ID3D12GraphicsCommandList* GetCommandList() { return CurrentGraphicsList; }
	void CreateCommandList();
	void Dispatch(int ThreadGroupCountX, int ThreadGroupCountY, int ThreadGroupCountZ) override;

	virtual void CopyResourceToSharedMemory(FrameBuffer* Buffer)override;
	virtual void CopyResourceFromSharedMemory(FrameBuffer* Buffer)override;
private:
	ID3D12GraphicsCommandList * CurrentGraphicsList = nullptr;
	bool IsOpen = false;
	D3D12Shader::PiplineShader				CurrentPipelinestate;
	ID3D12CommandAllocator* m_commandAllocator[RHI::CPUFrameCount];
	D3D12_INPUT_ELEMENT_DESC* VertexDesc = nullptr;
	int VertexDesc_ElementCount = 0;
	class D3D12Buffer* CurrentConstantBuffer = nullptr;
	class D3D12Texture* Texture = nullptr;
	class D3D12FrameBuffer* CurrentRenderTarget = nullptr;
	class D3D12FrameBuffer* CurrentFrameBufferTargets[10] = { nullptr };
	PipeLineState Currentpipestate;	
};

class D3D12Buffer : public RHIBuffer
{
public:
	D3D12Buffer(RHIBuffer::BufferType type, DeviceContext* Device = nullptr);
	virtual ~D3D12Buffer();
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	virtual void CreateConstantBuffer(int StructSize, int Elementcount) override;
	virtual void CreateVertexBuffer(int Stride, int ByteSize, BufferAccessType Accesstype = BufferAccessType::Static) override;
	void CreateStaticBuffer(int Stride, int ByteSize);
	void CreateDynamicBuffer(int Stride, int ByteSize);
	virtual void UpdateConstantBuffer(void * data, int offset) override;
	virtual void SetConstantBufferView(int offset, ID3D12GraphicsCommandList * list, int Register, bool IsCompute);
	//virtual void SetConstantBufferView(int offset, ID3D12GraphicsCommandList* list, int Register);
	virtual void UpdateIndexBuffer(void* data, int length) override;
	virtual void CreateIndexBuffer(int Stride, int ByteSize) override;
	void MapBuffer(void** Data);
	void UnMap();
	virtual void UpdateVertexBuffer(void* data, int length) override;
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
	bool CheckDevice(int index);
private:
	class D3D12CBV* CBV = nullptr;
	int ConstantBufferDataSize = 0;

	BufferAccessType BufferAccesstype;
	ID3D12Resource * m_vertexBuffer = nullptr;
	ID3D12Resource * m_indexBuffer = nullptr;
	ID3D12Resource * m_UploadBuffer = nullptr;
	int VertexBufferSize = 0;
	bool UploadComplete = false;

	DeviceContext* Device = nullptr;
};

class D3D12RHIUAV : public RHIUAV
{
public:
	D3D12RHIUAV( DeviceContext* inDevice);
	~D3D12RHIUAV();
	void CreateUAVFromTexture(class BaseTexture* target) override;
	void CreateUAVFromFrameBuffer(class FrameBuffer* target) override;
	void Bind(RHICommandList* list, int slot) override;
	ID3D12Resource * m_UAV = nullptr;
	DeviceContext* Device = nullptr;
	ID3D12Resource* UAVCounter = nullptr;
	class DescriptorHeap* Heap = nullptr;
};

class D3D12RHITextureArray : public RHITextureArray
{
public:
	//todo: Ensure Framebuffer srv matches!
	D3D12RHITextureArray(DeviceContext* device,int inNumEntries);
	virtual ~D3D12RHITextureArray() ;
	virtual void AddFrameBufferBind(FrameBuffer* Buffer, int slot)override;
	virtual void BindToShader(RHICommandList* list, int slot)override;
	virtual void SetIndexNull(int TargetIndex);
private:
	class DescriptorHeap* Heap = nullptr;
	std::vector<D3D12FrameBuffer*> LinkedBuffers;
	D3D12_SHADER_RESOURCE_VIEW_DESC NullHeapDesc = {};
	DeviceContext* Device  =nullptr;
};