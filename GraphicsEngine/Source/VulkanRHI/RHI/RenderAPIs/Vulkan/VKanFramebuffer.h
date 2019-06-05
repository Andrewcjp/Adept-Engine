#pragma once
#include "Rendering/Core/FrameBuffer.h"
#include "vulkan/vulkan_core.h"
#include "Descriptor.h"

#if BUILD_VULKAN
class VKanFramebuffer : public FrameBuffer
{
public:
	VKanFramebuffer(DeviceContext * device,const RHIFrameBufferDesc & Desc);


	// Inherited via FrameBuffer
	virtual DeviceContext * GetDevice() override;
	virtual const RHIPipeRenderTargetDesc & GetPiplineRenderDesc() override;
	virtual void MakeReadyForCopy(RHICommandList * list) override;
	void TryInitBuffer(RHIRenderPassDesc& desc,VKanCommandlist* list);

	Descriptor GetDescriptor(int slot);

	virtual void MakeReadyForComputeUse(RHICommandList* List, bool Depth = false) override;
	void UnBind(VKanCommandlist* List);
//private:
	RHIPipeRenderTargetDesc desc;
	bool IsCreated = false;
	VkFramebuffer Buffer;
	VkFramebuffer DepthBuffer;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	VkImage RTImage;
	VkDeviceMemory RTImageMemory;
	VkImageView RTImageView;

	VkImageView SRV;
};
#endif
