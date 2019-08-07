#include "RHIInterGPUStagingResource.h"
#include "Rendering\Core\FrameBuffer.h"


RHIInterGPUStagingResource::RHIInterGPUStagingResource(DeviceContext* owner, const InterGPUDesc& desc)
	:Desc(desc)
{
	OwnerDevice = owner;
}

RHIInterGPUStagingResource::~RHIInterGPUStagingResource()
{}

void RHIInterGPUStagingResource::SizeforBuffer(const RHIFrameBufferDesc & desc)
{

}

void RHIInterGPUStagingResource::SizeforFramebuffer(FrameBuffer * FB)
{
	SizeforBuffer(FB->GetDescription());
}

bool RHIInterGPUStagingResource::CanFitFB(FrameBuffer* FB)
{
	return false;
}

InterGPUDesc::InterGPUDesc()
{
	//by default avalable on all devices.
	Mask.SetFlags(0xffffff);
}
