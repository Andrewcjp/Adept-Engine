#pragma once
#include "Rendering/Core/FrameBuffer.h"
#include "vulkan/vulkan_core.h"

#if BUILD_VULKAN
class VKanFramebuffer : public FrameBuffer
{
public:
	VKanFramebuffer(DeviceContext * device,const RHIFrameBufferDesc & Desc);


	// Inherited via FrameBuffer
	virtual DeviceContext * GetDevice() override;
	virtual const RHIPipeRenderTargetDesc & GetPiplineRenderDesc() override;
	virtual void MakeReadyForCopy(RHICommandList * list) override;
	void TryInitBuffer(class VKanRenderPass* RenderPass);

	virtual void MakeReadyForComputeUse(RHICommandList* List, bool Depth = false) override;

private:
	RHIPipeRenderTargetDesc desc;
	bool IsCreated = false;
	VkFramebuffer Buffer;
	VkFramebuffer DepthBuffer;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;
};
#endif
