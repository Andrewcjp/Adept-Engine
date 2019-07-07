#pragma once
#include "RHITypes.h"
//Sits in Host memory the staging area for inter-gpu transfers 
//this is for one framebuffer in one frame but can be resized
class RHIInterGPUStagingResource: public IRHIResourse
{
public:
	RHI_API RHIInterGPUStagingResource(DeviceContext* owner);
	RHI_API ~RHIInterGPUStagingResource();
	RHI_API virtual void SizeforBuffer(const RHIFrameBufferDesc& desc);
	void SizeforFramebuffer(FrameBuffer* FB);
	RHI_API virtual void CopyFromFramebuffer(FrameBuffer* FB);
	RHI_API virtual void CopyToFramebuffer(FrameBuffer* FB);
	RHI_API virtual bool CanFitFB(FrameBuffer* FB);
protected:
	//Any device can access this though.
	DeviceContext* OwnerDevice = nullptr;
};

