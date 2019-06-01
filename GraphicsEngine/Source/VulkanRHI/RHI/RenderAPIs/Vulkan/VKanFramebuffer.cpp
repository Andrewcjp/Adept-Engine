
#include "VKanFramebuffer.h"
#if BUILD_VULKAN

VKanFramebuffer::VKanFramebuffer(DeviceContext * device, const RHIFrameBufferDesc & Desc) :FrameBuffer(device, Desc)
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

void VKanFramebuffer::MakeReadyForComputeUse(RHICommandList* List, bool Depth /*= false*/)
{
	throw std::logic_error("The method or operation is not implemented.");
}

void VKanFramebuffer::MakeReadyForCopy(RHICommandList * list)
{

}

void VKanFramebuffer::TryInitBuffer(VKanRenderPass * RenderPass)
{
	if (IsCreated)
	{
		return;
	}


	IsCreated = true;
}



#endif