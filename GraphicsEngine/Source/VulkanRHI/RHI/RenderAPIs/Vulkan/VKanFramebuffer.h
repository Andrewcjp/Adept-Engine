#pragma once
#include "Rendering/Core/FrameBuffer.h"
#include "vulkan/vulkan_core.h"
#include "Descriptor.h"
#include "Rendering/Core/GPUStateCache.h"

class VknGPUResource;
class VKanCommandlist;

#if BUILD_VULKAN
class VKanFramebuffer : public FrameBuffer
{
public:
	VKanFramebuffer(DeviceContext * device, const RHIFrameBufferDesc & Desc);


	// Inherited via FrameBuffer
	virtual DeviceContext * GetDevice() override;
	virtual const RHIPipeRenderTargetDesc & GetPiplineRenderDesc() override;
	virtual void MakeReadyForCopy(RHICommandList * list) override;
	void TryInitBuffer(RHIRenderPassDesc& desc, VKanCommandlist* list);

	void CreateRT(VKanCommandlist* list, int index);

	void UpdateStateTrackingFromRP(RHIRenderPassDesc & Desc);
	Descriptor GetDescriptor(int slot, int resourceindex);

	virtual void MakeReadyForComputeUse(RHICommandList* List, bool Depth = false) override;
	void UnBind(VKanCommandlist* List);
	void TransitionTOPixel(VKanCommandlist * list);
	//private:
	RHIPipeRenderTargetDesc desc;
	bool IsCreated = false;
	VkFramebuffer Buffer;
	VkFramebuffer DepthBuffer;
	bool WasTexture = false;

	VkImageView depthImageView;
	VkImageView RTImageView[MAX_MRTS];

	VkImageView SRV;

	VknGPUResource* DepthResource = nullptr;
	VknGPUResource* RTImages[MAX_MRTS] = {nullptr};

};
#endif
