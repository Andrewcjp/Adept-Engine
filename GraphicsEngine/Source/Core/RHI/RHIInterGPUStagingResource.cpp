#include "RHIInterGPUStagingResource.h"
#include "Rendering\Core\FrameBuffer.h"


RHIInterGPUStagingResource::RHIInterGPUStagingResource(DeviceContext* owner, const InterGPUDesc& desc)
{
	OwnerDevice = owner;
	Desc = desc;
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
	Mask.SetFlags(0xffffff);
}
