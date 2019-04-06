#pragma once
#include "RHI/RHICommandList.h"
#include "RHI/Shader.h"
#include "VKanRHI.h"
#include "vulkan/vulkan_core.h"
#if BUILD_VULKAN
class VKanCommandlist :
	public RHICommandList
{
public:
	VKanCommandlist(ECommandListType::Type type, DeviceContext* context);
	~VKanCommandlist();

	// Inherited via RHICommandList
	virtual void ResetList() override;

	virtual void SetViewport(int MinX, int MinY, int MaxX, int MaxY, float MaxZ, float MinZ) override;

	virtual void DrawPrimitive(int VertexCountPerInstance, int InstanceCount, int StartVertexLocation, int StartInstanceLocation) override;
	virtual void DrawIndexedPrimitive(int IndexCountPerInstance, int InstanceCount, int StartIndexLocation, int BaseVertexLocation, int StartInstanceLocation) override;
	virtual void SetVertexBuffer(RHIBuffer * buffer) override;

	virtual void UpdateConstantBuffer(void * data, int offset) override;
	virtual void SetConstantBufferView(RHIBuffer * buffer, int offset, int Register) override;
	virtual void SetScreenBackBufferAsRT() override;
	virtual void ClearScreen() override;
	virtual void ClearFrameBuffer(FrameBuffer * buffer) override;
	virtual void UAVBarrier(RHIUAV * target) override;

	// Inherited via RHICommandList
	virtual void SetIndexBuffer(RHIBuffer * buffer) override;

	// Inherited via RHICommandList
	virtual void Dispatch(int ThreadGroupCountX, int ThreadGroupCountY, int ThreadGroupCountZ) override;

	// Inherited via RHICommandList
	virtual void SetRenderTarget(FrameBuffer * target, int SubResourceIndex = 0) override;
	virtual void Execute(DeviceContextQueue::Type Target = DeviceContextQueue::LIMIT) override;
	virtual void SetFrameBufferTexture(FrameBuffer * buffer, int slot, int Resourceindex = 0) override;
	virtual void SetUpCommandSigniture(int commandSize, bool Dispatch) override;
	virtual void ExecuteIndiect(int MaxCommandCount, RHIBuffer * ArgumentBuffer, int ArgOffset, RHIBuffer * CountBuffer, int CountBufferOffset) override;
	virtual void SetRootConstant(int SignitureSlot, int ValueNum, void * Data, int DataOffset) override;
	VkCommandBuffer CommandBuffer;

	RHI_VIRTUAL void SetPipelineStateDesc(RHIPipeLineStateDesc& Desc) override;


	RHI_VIRTUAL void SetPipelineStateObject(RHIPipeLineStateObject* Object) override;


	RHI_VIRTUAL void SetTexture(class BaseTexture* texture, int slot) override;
	VkCommandBuffer* GetCommandBuffer();

	RHI_VIRTUAL void BeginRenderPass(RHIRenderPassInfo& RenderPass) override;
	RHI_VIRTUAL void EndRenderPass() override;

private:

	struct CPUFrame
	{
		VkCommandPool Pool = nullptr;
		VkCommandBuffer Buffer = nullptr;
	};
	CPUFrame Pools[RHI::CPUFrameCount] = { 0 };
	VKanRenderPass* CurrnetRenderPass = nullptr;
};


class VkanUAV :public RHIUAV
{
public:
	VkanUAV()
	{};
	// Inherited via RHIUAV
	virtual void Bind(RHICommandList * list, int slot) override;
	virtual void CreateUAVFromFrameBuffer(FrameBuffer * target) override;
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
};


#endif