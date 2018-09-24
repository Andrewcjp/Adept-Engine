#include "stdafx.h"
#include "VKanFramebuffer.h"
#if BUILD_VULKAN

#endif

VKanFramebuffer::VKanFramebuffer(DeviceContext * device, RHIFrameBufferDesc & Desc) :FrameBuffer(device, Desc)
{

}
DeviceContext * VKanFramebuffer::GetDevice()
{
	return nullptr;
}

const RHIPipeRenderTargetDesc & VKanFramebuffer::GetPiplineRenderDesc()
{
	// TODO: insert return statement here
	return desc;
}

void VKanFramebuffer::MakeReadyForComputeUse(RHICommandList * List)
{}
