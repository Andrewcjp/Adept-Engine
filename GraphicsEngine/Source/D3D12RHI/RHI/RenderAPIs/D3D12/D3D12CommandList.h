#pragma once
#include "RHI/RHICommandList.h"
#include "D3D12Shader.h"
#include "Core/Utils/RefChecker.h"
#include "RHI/BaseTexture.h"

class DescriptorGroup;
class CommandAllocator;
class D3D12PipeLineStateObject :public RHIPipeLineStateObject
{
public:
	D3D12PipeLineStateObject(const RHIPipeLineStateDesc& desc, DeviceContext* con);
	~D3D12PipeLineStateObject();
	virtual void Complie() override;
	ID3D12RootSignature* RootSig = nullptr;
	ID3D12PipelineState* PSO = nullptr;

	virtual void Release() override;

};
typedef ID3D12GraphicsCommandList CMDListType;

class D3D12CommandList : public RHICommandList
{
public:
	D3D12CommandList(DeviceContext * inDevice, ECommandListType::Type ListType = ECommandListType::Graphics);

	virtual ~D3D12CommandList();

	// Inherited via RHICommandList
	virtual void ResetList() override;
	ID3D12CommandAllocator * GetCommandAllocator();
	void SetRenderTarget(FrameBuffer * target, int SubResourceIndex = 0);
	virtual void DrawPrimitive(int VertexCountPerInstance, int InstanceCount, int StartVertexLocation, int StartInstanceLocation) override;
	virtual void DrawIndexedPrimitive(int IndexCountPerInstance, int InstanceCount, int StartIndexLocation, int BaseVertexLocation, int StartInstanceLocation) override;
	virtual void SetViewport(int MinX, int MinY, int MaxX, int MaxY, float MaxZ, float MinZ) override;
	virtual void Execute(DeviceContextQueue::Type Target = DeviceContextQueue::LIMIT) override;
	virtual void SetVertexBuffer(RHIBuffer * buffer) override;
	virtual void SetIndexBuffer(RHIBuffer* buffer) override;
	virtual void SetPipelineStateObject(RHIPipeLineStateObject* Object) override;
	void PushState();
	virtual void SetConstantBufferView(RHIBuffer * buffer, int offset, int Register) override;
	virtual void SetTexture(BaseTextureRef texture, int slot) override;
	virtual void SetFrameBufferTexture(FrameBuffer * buffer, int slot, int Resourceindex = 0) override;

	virtual void ClearFrameBuffer(FrameBuffer * buffer) override;
	virtual void UAVBarrier(class RHIUAV* target) override;
	virtual void SetUpCommandSigniture(int commandSize, bool Dispatch) override;

	virtual void SetRootConstant(int SignitureSlot, int ValueNum, void* Data, int DataOffset);
	CMDListType* GetCommandList();
	void CreateCommandList();
	void Dispatch(int ThreadGroupCountX, int ThreadGroupCountY, int ThreadGroupCountZ) override;

	virtual void CopyResourceToSharedMemory(FrameBuffer* Buffer)override;
	virtual void CopyResourceFromSharedMemory(FrameBuffer* Buffer)override;
	void Release()override;
	bool IsOpen();
	virtual void ExecuteIndiect(int MaxCommandCount, RHIBuffer* ArgumentBuffer, int ArgOffset, RHIBuffer* CountBuffer, int CountBufferOffset);

	virtual void SetPipelineStateDesc(RHIPipeLineStateDesc& Desc) override;
	class D3D12FrameBuffer* CurrentRenderTarget = nullptr;
	RHI_VIRTUAL void BeginRenderPass(RHIRenderPassDesc& RenderPass) override;
	RHI_VIRTUAL void EndRenderPass() override;
	void AddHeap(DescriptorHeap* heap);
	void PushHeaps();
	void ClearHeaps();
#if AFTERMATH
	GFSDK_Aftermath_ContextHandle AMHandle;
#endif
	ID3D12GraphicsCommandList4* GetCMDList4();
	virtual void TraceRays(const RHIRayDispatchDesc& desc) override;
	virtual void SetHighLevelAccelerationStructure(HighLevelAccelerationStructure* Struct) override;
	virtual void SetStateObject(RHIStateObject* Object) override;

	 RHI_VIRTUAL void SetDepthBounds(float Min, float Max) override;


	 virtual void BindSRV(FrameBuffer* Buffer, int slot, RHIViewDesc Desc) override;

private:
	void SetScreenBackBufferAsRT();
	void ClearScreen();
	D3D12StateObject* CurrentRTState = nullptr;
	std::vector<DescriptorHeap*> heaps;
	void PushPrimitiveTopology();
	class D3D12DeviceContext* mDeviceContext = nullptr;
	CMDListType* CurrentCommandList = nullptr;
	ID3D12GraphicsCommandList4* CurrentADVCommandList = nullptr;
	ID3D12GraphicsCommandList1* CommandList1 = nullptr;
	bool m_IsOpen = false;
	//ID3D12CommandAllocator* m_commandAllocator[RHI::CPUFrameCount];
	D3D12_INPUT_ELEMENT_DESC VertexDesc = D3D12_INPUT_ELEMENT_DESC();
	std::vector<ShaderParameter> Params;
	int VertexDesc_ElementCount = 0;
	class D3D12Buffer* CurrentConstantBuffer = nullptr;
	class D3D12Texture* Texture = nullptr;
	CommandAllocator* CommandAlloc = nullptr;
	class D3D12FrameBuffer* CurrentFrameBufferTargets[10] = { nullptr };

	ID3D12CommandSignature* CommandSig = nullptr;
};

class D3D12RHIUAV : public RHIUAV
{
public:
	D3D12RHIUAV(DeviceContext* inDevice);

	~D3D12RHIUAV();
	void CreateUAVFromTexture(class BaseTexture* target) override;
	RHI_VIRTUAL void CreateUAVFromFrameBuffer(class FrameBuffer* target, RHIViewDesc desc = RHIViewDesc()) override;
	void Bind(RHICommandList* list, int slot) override;
	ID3D12Resource * m_UAV = nullptr;
	D3D12DeviceContext* Device = nullptr;
	ID3D12Resource* UAVCounter = nullptr;
	DescriptorGroup* UAVDescriptor = nullptr;
protected:
	void Release() override;
	virtual void CreateUAVFromRHIBuffer(RHIBuffer * target) override;
};



class D3D12RHITextureArray : public RHITextureArray
{
public:
	//#RHI: Ensure Framebuffer srv matches!
	D3D12RHITextureArray(DeviceContext* device, int inNumEntries);
	virtual ~D3D12RHITextureArray();
	RHI_VIRTUAL void AddFrameBufferBind(FrameBuffer* Buffer, int slot)override;
	RHI_VIRTUAL void BindToShader(RHICommandList* list, int slot)override;
	RHI_VIRTUAL void SetIndexNull(int TargetIndex, FrameBuffer* Buffer = nullptr);
	RHI_VIRTUAL void SetFrameBufferFormat(RHIFrameBufferDesc & desc);
private:
	void Release() override;
	void Clear() override;
	class DescriptorGroup* Desc = nullptr;
	std::vector<D3D12FrameBuffer*> LinkedBuffers;
	D3D12_SHADER_RESOURCE_VIEW_DESC NullHeapDesc = {};
	D3D12DeviceContext* Device = nullptr;
};
CreateChecker(D3D12CommandList);
CreateChecker(D3D12RHITextureArray);
CreateChecker(D3D12RHIUAV);
