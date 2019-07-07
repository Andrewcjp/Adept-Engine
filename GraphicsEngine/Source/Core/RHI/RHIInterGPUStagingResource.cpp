#include "RHIInterGPUStagingResource.h"
#include "Rendering\Core\FrameBuffer.h"


RHIInterGPUStagingResource::RHIInterGPUStagingResource(DeviceContext* owner)
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

void RHIInterGPUStagingResource::CopyFromFramebuffer(FrameBuffer * FB)
{}

void RHIInterGPUStagingResource::CopyToFramebuffer(FrameBuffer * FB)
{}

bool RHIInterGPUStagingResource::CanFitFB(FrameBuffer* FB)
{
	return false;
}
