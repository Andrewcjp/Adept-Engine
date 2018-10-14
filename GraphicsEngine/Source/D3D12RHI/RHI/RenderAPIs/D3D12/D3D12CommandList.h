#pragma once
#include "RHI/RHICommandList.h"
#include <d3d12.h>
#include "D3D12Shader.h"
#include "Core/Utils/RefChecker.h"
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
	virtual void Execute(DeviceContextQueue::Type Target = DeviceContextQueue::LIMIT) override;
	virtual void SetVertexBuffer(RHIBuffer * buffer) override;
	virtual void SetIndexBuffer(RHIBuffer* buffer) override;
	virtual void SetPipelineState(PipeLineState state) override;
	virtual void CreatePipelineState(class Shader * shader, class FrameBuffer* Buffer = nullptr) override;

	
	
	virtual void SetPipelineStateObject(class Shader* shader, class FrameBuffer* Buffer = nullptr)override;
	void IN_CreatePipelineState(Shader * shader);

	virtual void UpdateConstantBuffer(void * data, int offset) override;
	virtual void SetConstantBufferView(RHIBuffer * buffer, int offset, int Register) override;
	virtual void SetTexture(class BaseTexture * texture, int slot) override;
	virtual void SetFrameBufferTexture(FrameBuffer * buffer, int slot, int Resourceindex = 0) override;

	virtual void SetScreenBackBufferAsRT() override;
	virtual void ClearScreen() override;
	virtual void ClearFrameBuffer(FrameBuffer * buffer) override;
	virtual void UAVBarrier(class RHIUAV* target) override;
	virtual void SetUpCommandSigniture(int commandSize, bool Dispatch) override;

	virtual void SetRootConstant(int SignitureSlot, int ValueNum, void* Data, int DataOffset);
	ID3D12GraphicsCommandList* GetCommandList();
	void CreateCommandList();
	void Dispatch(int ThreadGroupCountX, int ThreadGroupCountY, int ThreadGroupCountZ) override;

	virtual void CopyResourceToSharedMemory(FrameBuffer* Buffer)override;
	virtual void CopyResourceFromSharedMemory(FrameBuffer* Buffer)override;
	void Release()override;
	bool IsOpen()
	{
		return m_IsOpen;
	}
	virtual void ExecuteIndiect(int MaxCommandCount, RHIBuffer* ArgumentBuffer, int ArgOffset, RHIBuffer* CountBuffer, int CountBufferOffset);

private:
	std::string GetPSOHash(Shader * shader, const RHIPipeRenderTargetDesc & statedesc);
	void PushPrimitiveTopology();
	class D3D12DeviceContext* mDeviceContext = nullptr;
	ID3D12GraphicsCommandList * CurrentCommandList = nullptr;
	bool m_IsOpen = false;
	D3D12PiplineShader				CurrentPipelinestate;
	ID3D12CommandAllocator* m_commandAllocator[RHI::CPUFrameCount];
	D3D12_INPUT_ELEMENT_DESC VertexDesc = D3D12_INPUT_ELEMENT_DESC();
	std::vector<Shader::ShaderParameter> Params;
	int VertexDesc_ElementCount = 0;
	class D3D12Buffer* CurrentConstantBuffer = nullptr;
	class D3D12Texture* Texture = nullptr;
	class D3D12FrameBuffer* CurrentRenderTarget = nullptr;
	class D3D12FrameBuffer* CurrentFrameBufferTargets[10] = { nullptr };
	PipeLineState Currentpipestate;
	std::map<std::string, D3D12PiplineShader> PSOCache;

	ID3D12CommandSignature* CommandSig = nullptr;
	std::string CurrnetPsoKey = "";
};

class D3D12RHIUAV : public RHIUAV
{
public:
	D3D12RHIUAV(DeviceContext* inDevice);

	~D3D12RHIUAV();
	void CreateUAVFromTexture(class BaseTexture* target) override;
	void CreateUAVFromFrameBuffer(class FrameBuffer* target) override;
	void Bind(RHICommandList* list, int slot) override;
	ID3D12Resource * m_UAV = nullptr;
	D3D12DeviceContext* Device = nullptr;
	ID3D12Resource* UAVCounter = nullptr;
	class DescriptorHeap* Heap = nullptr;
protected:
	void Release() override;
	virtual void CreateUAVFromRHIBuffer(RHIBuffer * target) override;
};

class D3D12Buffer : public RHIBuffer
{
public:
	D3D12Buffer(ERHIBufferType::Type type, DeviceContext* Device = nullptr);
	virtual ~D3D12Buffer();
	virtual void CreateConstantBuffer(int StructSize, int Elementcount, bool ReplicateToAllDevices = false) override;
	virtual void CreateVertexBuffer(int Stride, int ByteSize, EBufferAccessType::Type Accesstype = EBufferAccessType::Static) override;
	virtual void UpdateConstantBuffer(void * data, int offset) override;
	virtual void UpdateIndexBuffer(void* data, size_t length) override;
	virtual void UpdateBufferData(void * data, size_t length, EBufferResourceState::Type state) override;
	virtual void CreateIndexBuffer(int Stride, int ByteSize) override;
	virtual void CreateBuffer(RHIBufferDesc desc) override;
	virtual void UpdateVertexBuffer(void* data, size_t length) override;
	virtual void BindBufferReadOnly(RHICommandList* list, int RSSlot)override;
	virtual void SetBufferState(class RHICommandList* list, EBufferResourceState::Type State) override;
	bool CheckDevice(int index);
	void EnsureResouceInFinalState(ID3D12GraphicsCommandList* list);
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	void SetConstantBufferView(int offset, ID3D12GraphicsCommandList * list, int Slot, bool IsCompute, int Deviceindex);
	class GPUResource* GetResource();

protected:
	void UpdateData(void * data, size_t length, D3D12_RESOURCE_STATES EndState);
	void Release() override;
	void SetupBufferSRV();
	void CreateUAV();
	friend class D3D12RHIUAV;
private:
	void MapBuffer(void** Data);
	void UnMap();
	void CreateStaticBuffer(int ByteSize);
	void CreateDynamicBuffer(int ByteSize);

	class D3D12CBV* CBV[MAX_GPU_DEVICE_COUNT][RHI::CPUFrameCount] = { nullptr };
	EBufferAccessType::Type BufferAccesstype;
	ID3D12Resource * m_UploadBuffer = nullptr;
	GPUResource* m_DataBuffer = nullptr;
	int ElementCount = 0;
	int ElementSize = 0;
	bool UploadComplete = false;
	bool CrossDevice = false;
	D3D12DeviceContext* Device = nullptr;
	class DescriptorHeap* SRVBufferHeap = nullptr;
	D3D12_RESOURCE_STATES PostUploadState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_COMMON;
};


class D3D12RHITextureArray : public RHITextureArray
{
public:
	//todo: Ensure Framebuffer srv matches!
	D3D12RHITextureArray(DeviceContext* device, int inNumEntries);
	virtual ~D3D12RHITextureArray();
	virtual void AddFrameBufferBind(FrameBuffer* Buffer, int slot)override;
	virtual void BindToShader(RHICommandList* list, int slot)override;
	virtual void SetIndexNull(int TargetIndex);
private:
	void Release() override;
	class DescriptorHeap* Heap = nullptr;
	std::vector<D3D12FrameBuffer*> LinkedBuffers;
	D3D12_SHADER_RESOURCE_VIEW_DESC NullHeapDesc = {};
	D3D12DeviceContext* Device = nullptr;
};
CreateChecker(D3D12CommandList);
CreateChecker(D3D12Buffer);
CreateChecker(D3D12RHITextureArray);
CreateChecker(D3D12RHIUAV);
