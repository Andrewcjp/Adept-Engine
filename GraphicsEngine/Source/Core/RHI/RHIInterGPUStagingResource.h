#pragma once

//Sits in Host memory the staging area for inter-gpu transfers 
//this is for one framebuffer in one frame but can be resized
struct InterGPUDesc
{
	GPUDeviceMask Mask;
	RHIFrameBufferDesc FramebufferDesc;
	bool IsBuffer = false;
	RHIBufferDesc BufferDesc;
	InterGPUDesc();
};
class RHIInterGPUStagingResource: public IRHIResourse
{
public:
	RHI_API RHIInterGPUStagingResource(DeviceContext* owner,const InterGPUDesc& desc);
	RHI_API ~RHIInterGPUStagingResource();
	RHI_API virtual void SizeforBuffer(const RHIFrameBufferDesc& desc);
	void SizeforFramebuffer(FrameBuffer* FB);
	RHI_API virtual bool CanFitFB(FrameBuffer* FB);
protected:
	//Any device can access this though.
	DeviceContext* OwnerDevice = nullptr;
	InterGPUDesc Desc;
};

