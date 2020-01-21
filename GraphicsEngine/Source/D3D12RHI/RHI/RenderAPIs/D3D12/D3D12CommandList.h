#pragma once
#include "RHI/RHICommandList.h"
#include "D3D12Shader.h"
#include "Core/Utils/RefChecker.h"
#include "RHI/BaseTexture.h"
#include "RHI/RHIRootSigniture.h"

class DescriptorGroup;
class CommandAllocator;
class D3D12CommandSigniture;
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
	RHI_VIRTUAL void DrawIndexedPrimitive(uint IndexCountPerInstance, uint InstanceCount, uint StartIndexLocation, uint BaseVertexLocation, uint StartInstanceLocation) override;
	virtual void SetViewport(int MinX, int MinY, int MaxX, int MaxY, float MaxZ, float MinZ) override;
	virtual void Execute(DeviceContextQueue::Type Target = DeviceContextQueue::LIMIT) override;
	virtual void SetVertexBuffer(RHIBuffer * buffer) override;
	virtual void SetIndexBuffer(RHIBuffer* buffer) override;
	virtual void SetPipelineStateObject(RHIPipeLineStateObject* Object) override;
	void PushState();
	virtual void SetConstantBufferView(RHIBuffer * buffer, int offset, int Register) override;
	void PrepareforDraw();
	virtual void SetTexture(BaseTextureRef texture, int slot, const RHIViewDesc & desc) override;
	virtual void SetFrameBufferTexture(FrameBuffer * buffer, int slot, const RHIViewDesc & desc) override;

	virtual void ClearFrameBuffer(FrameBuffer * buffer) override;
	virtual void SetCommandSigniture(RHICommandSignitureDescription desc)override;
	virtual void SetRootConstant(int SignitureSlot, int ValueNum, void* Data, int DataOffset);
	CMDListType* GetCommandList();
	void CreateCommandList();
	void Dispatch(int ThreadGroupCountX, int ThreadGroupCountY, int ThreadGroupCountZ) override;

	virtual void CopyResourceToSharedMemory(FrameBuffer* Buffer)override;
	virtual void CopyResourceFromSharedMemory(FrameBuffer* Buffer)override;
	void Release()override;
	bool IsOpen() const override;
	virtual void ExecuteIndiect(int MaxCommandCount, RHIBuffer* ArgumentBuffer, int ArgOffset, RHIBuffer* CountBuffer, int CountBufferOffset);

	virtual void SetPipelineStateDesc(const RHIPipeLineStateDesc& Desc) override;
	class D3D12FrameBuffer* CurrentRenderTarget = nullptr;
	RHI_VIRTUAL void BeginRenderPass(const RHIRenderPassDesc& RenderPass) override;
	RHI_VIRTUAL void EndRenderPass() override;
	void AddHeap(DescriptorHeap* heap);
	void PushHeaps();
	void ClearHeaps();
#if AFTERMATH
	GFSDK_Aftermath_ContextHandle AMHandle;
#endif
#if WIN10_1809
	ID3D12GraphicsCommandList4* GetCMDList4();
#endif
#if WIN10_1809
	virtual void TraceRays(const RHIRayDispatchDesc& desc) override;
	virtual void SetHighLevelAccelerationStructure(HighLevelAccelerationStructure* Struct) override;
	virtual void SetStateObject(RHIStateObject* Object) override;
#endif
#if WIN10_1903
	ID3D12GraphicsCommandList5* GetCMDList5();
#endif
	RHI_VIRTUAL void SetDepthBounds(float Min, float Max) override;
	RHI_VIRTUAL void SetConstantBufferView(RHIBuffer * buffer, RHIViewDesc Desc, int Slot) override;
	RHI_VIRTUAL void SetBuffer(RHIBuffer* Buffer, int slot, const RHIViewDesc & desc) override;
	RHI_VIRTUAL void SetTextureArray(RHITextureArray* array, int slot, const RHIViewDesc& view) override;
	RHI_VIRTUAL void SetUAV(RHIBuffer* buffer, int slot, const RHIViewDesc & view) override;
	RHI_VIRTUAL void SetUAV(RHITexture * buffer, int slot, const RHIViewDesc & view) override;
	RHI_VIRTUAL void SetUAV(FrameBuffer* buffer, int slot, const RHIViewDesc & view = RHIViewDesc()) override;
	RHI_VIRTUAL void UAVBarrier(FrameBuffer* target) override;
	RHI_VIRTUAL void UAVBarrier(RHIBuffer* target) override;
	RHIRootSigniture* GetRootSig();

	RHI_VIRTUAL void SetTexture2(RHITexture* t, int slot, const RHIViewDesc& view) override;


	RHI_VIRTUAL void SetStencilRef(uint value) override;
	RHI_VIRTUAL void FlushBarriers() override;
	void AddTransition(D3D12_RESOURCE_BARRIER transition);

	RHI_VIRTUAL void ClearUAVFloat(RHIBuffer* buffer) override;


	RHI_VIRTUAL void ClearUAVUint(RHIBuffer* buffer) override;

	void CopyResource(RHITexture* Source, RHITexture* Dest) override;

private:
	void SetScreenBackBufferAsRT();
	void ClearScreen();
	D3D12StateObject* CurrentRTState = nullptr;
	std::vector<DescriptorHeap*> heaps;
	void PushPrimitiveTopology();
	class D3D12DeviceContext* mDeviceContext = nullptr;
	CMDListType* CurrentCommandList = nullptr;
#if WIN10_1809
	ID3D12GraphicsCommandList4* CurrentADVCommandList = nullptr;
#endif
#if WIN10_1903
	ID3D12GraphicsCommandList5* CmdList5 = nullptr;
#endif
	ID3D12GraphicsCommandList1* CommandList1 = nullptr;
	bool m_IsOpen = false;
	//ID3D12CommandAllocator* m_commandAllocator[RHI::CPUFrameCount];
	D3D12_INPUT_ELEMENT_DESC VertexDesc = D3D12_INPUT_ELEMENT_DESC();

	int VertexDesc_ElementCount = 0;
	class D3D12Buffer* CurrentConstantBuffer = nullptr;
	class D3D12Texture* Texture = nullptr;
	CommandAllocator* CommandAlloc = nullptr;
	class D3D12FrameBuffer* CurrentFrameBufferTargets[10] = { nullptr };
	D3D12CommandSigniture* CommandSig = nullptr;
	RHIRootSigniture RootSigniture;
	ID3D12DescriptorHeap* CurrentBoundHeap = nullptr;
	std::vector<D3D12_RESOURCE_BARRIER> QueuedBarriers;
	//value based on shaders + size of screen/dispatch.
	uint GPUWorkEstimate = 0;
	//raw number of CPU commands
	uint CommandCount = 0;
	//number of commands that launch CUs
	uint DrawDispatchCount = 0;
protected:
#if WIN10_1903
	virtual void SetVRSShadingRateNative(VRX_SHADING_RATE::type Rate) override;
	virtual void SetVRSShadingRateImageNative(RHITexture* Target) override;
#endif
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
	RHI_VIRTUAL void SetFrameBufferFormat(const RHIFrameBufferDesc & desc);
	DXDescriptor* GetDescriptor(const RHIViewDesc & desc, DescriptorHeap* heap = nullptr);
	uint64 GetHash();
private:
	void Release() override;
	void Clear() override;
	std::vector<D3D12FrameBuffer*> LinkedBuffers;
	D3D12_SHADER_RESOURCE_VIEW_DESC NullHeapDesc = {};
	D3D12DeviceContext* Device = nullptr;
};
CreateChecker(D3D12CommandList);
CreateChecker(D3D12RHITextureArray);
CreateChecker(D3D12RHIUAV);


class D3D12CommandSigniture :public RHICommandSigniture
{
public:
	D3D12CommandSigniture(DeviceContext* context, RHICommandSignitureDescription desc = RHICommandSignitureDescription());
	ID3D12CommandSignature* GetSigniture();
	virtual void Build() override;

	virtual void Release() override;

private:
	ID3D12CommandSignature* CommandSig = nullptr;
};