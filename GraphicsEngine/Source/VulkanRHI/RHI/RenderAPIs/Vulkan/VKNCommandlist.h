#pragma once
#include "RHI/RHICommandList.h"
#include "RHI/Shader.h"
#include "VKNRHI.h"
#include "vulkan/vulkan_core.h"
#include "RHI/RHIRootSigniture.h"

class Descriptor;
#if BUILD_VULKAN
class VKNCommandlist :
	public RHICommandList
{
public:
	VKNCommandlist(ECommandListType::Type type, DeviceContext* context);
	~VKNCommandlist();

	// Inherited via RHICommandList
	virtual void ResetList() override;

	virtual void SetViewport(int MinX, int MinY, int MaxX, int MaxY, float MaxZ, float MinZ) override;

	virtual void DrawPrimitive(int VertexCountPerInstance, int InstanceCount, int StartVertexLocation, int StartInstanceLocation) override;
	virtual void DrawIndexedPrimitive(int IndexCountPerInstance, int InstanceCount, int StartIndexLocation, int BaseVertexLocation, int StartInstanceLocation) override;
	virtual void SetVertexBuffer(RHIBuffer * buffer) override;
	virtual void SetConstantBufferView(RHIBuffer * buffer, int offset, int Register) override;
	virtual void ClearFrameBuffer(FrameBuffer * buffer) override;
	virtual void UAVBarrier(RHIUAV * target) override;
	virtual void SetIndexBuffer(RHIBuffer * buffer) override;
	virtual void Dispatch(int ThreadGroupCountX, int ThreadGroupCountY, int ThreadGroupCountZ) override;

	virtual void Execute(DeviceContextQueue::Type Target = DeviceContextQueue::LIMIT) override;
	virtual void SetFrameBufferTexture(FrameBuffer * buffer, int slot, int Resourceindex = 0) override;
	virtual void ExecuteIndiect(int MaxCommandCount, RHIBuffer * ArgumentBuffer, int ArgOffset, RHIBuffer * CountBuffer, int CountBufferOffset) override;
	virtual void SetRootConstant(int SignitureSlot, int ValueNum, void * Data, int DataOffset) override;
	RHI_VIRTUAL void SetPipelineStateDesc(RHIPipeLineStateDesc& Desc) override;
	RHI_VIRTUAL void SetPipelineStateObject(RHIPipeLineStateObject* Object) override;

	RHI_VIRTUAL void BeginRenderPass(RHIRenderPassDesc& RenderPass) override;
	RHI_VIRTUAL void EndRenderPass() override;
	RHI_VIRTUAL void SetTexture(BaseTextureRef texture, int slot) override;
	virtual void SetHighLevelAccelerationStructure(HighLevelAccelerationStructure* Struct) override;
	virtual void TraceRays(const RHIRayDispatchDesc& desc) override;
	virtual void SetStateObject(RHIStateObject* Object) override;
	VkCommandBuffer* GetCommandBuffer();
	//private:
	VkCommandBuffer CommandBuffer;
	struct CPUFrame
	{
		VkCommandPool Pool = nullptr;
		VkCommandBuffer Buffer = nullptr;
	};
	CPUFrame Pools[RHI::CPUFrameCount] = { 0 };
	VKNRenderPass* CurrnetRenderPass = nullptr;
	VKNPipeLineStateObject* CurrentPso = nullptr;
	bool IsOpen = false;
	 RHI_VIRTUAL void SetDepthBounds(float Min, float Max) override;


	 virtual void BindSRV(FrameBuffer* Buffer, int slot, RHIViewDesc Desc) override;
	 RHIRootSigniture Rootsig;
};


class VkanUAV :public RHIUAV
{
public:
	VkanUAV()
	{};
	// Inherited via RHIUAV
	virtual void Bind(RHICommandList * list, int slot) override;
	virtual void CreateUAVFromFrameBuffer(class FrameBuffer* target, RHIViewDesc desc = RHIViewDesc()) override;
	virtual void CreateUAVFromTexture(BaseTexture * target) override;
	virtual void CreateUAVFromRHIBuffer(RHIBuffer * target) override;
};
class VkanTextureArray :public RHITextureArray
{
public:
	VkanTextureArray(DeviceContext* device, int inNumEntries) :RHITextureArray(device, inNumEntries)
	{};
	// Inherited via RHITextureArray
	virtual void AddFrameBufferBind(FrameBuffer * Buffer, int slot) override;
	virtual void BindToShader(RHICommandList * list, int slot) override;
	RHI_VIRTUAL void SetIndexNull(int TargetIndex, FrameBuffer* Buffer = nullptr) override;
	RHI_VIRTUAL void Clear() override;
	RHI_VIRTUAL void SetFrameBufferFormat(RHIFrameBufferDesc & desc) override;
private:
	FrameBuffer* Tmp;
};


#endif