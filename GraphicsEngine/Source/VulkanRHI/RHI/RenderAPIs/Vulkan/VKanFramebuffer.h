#pragma once
#include "Rendering/Core/FrameBuffer.h"
#if BUILD_VULKAN
class VKanFramebuffer : public FrameBuffer
{
public:
	VKanFramebuffer(DeviceContext * device, RHIFrameBufferDesc & Desc);


	// Inherited via FrameBuffer
	virtual DeviceContext * GetDevice() override;

	virtual const RHIPipeRenderTargetDesc & GetPiplineRenderDesc() override;

	virtual void MakeReadyForComputeUse(RHICommandList * List) override;
private:
	RHIPipeRenderTargetDesc desc;
};
#endif
