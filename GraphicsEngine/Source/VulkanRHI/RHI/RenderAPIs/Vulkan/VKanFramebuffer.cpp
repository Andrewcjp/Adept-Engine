
#include "VKanFramebuffer.h"
#if BUILD_VULKAN

VKanFramebuffer::VKanFramebuffer(DeviceContext * device,const RHIFrameBufferDesc & Desc) :FrameBuffer(device, Desc)
{

}
DeviceContext * VKanFramebuffer::GetDevice()
{
	return nullptr;
}

const RHIPipeRenderTargetDesc & VKanFramebuffer::GetPiplineRenderDesc()
{
	return desc;
}

void VKanFramebuffer::MakeReadyForComputeUse(RHICommandList * List)
{}

void VKanFramebuffer::MakeReadyForCopy(RHICommandList * list)
{}
void VKanFramebuffer::TryInitBuffer(VKanRenderPass * RenderPass)
{}
#endif