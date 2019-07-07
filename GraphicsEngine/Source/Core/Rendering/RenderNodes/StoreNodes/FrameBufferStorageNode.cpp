#include "FrameBufferStorageNode.h"
#include "Rendering/Core/FrameBuffer.h"

FrameBufferStorageNode::FrameBufferStorageNode()
{
	StoreType = EStorageType::Framebuffer;
}

FrameBufferStorageNode::~FrameBufferStorageNode()
{
	SafeRHIRelease(FBuffer);
}

void FrameBufferStorageNode::SetFrameBufferDesc(RHIFrameBufferDesc & desc)
{
	FramebufferDesc = desc;
}

const RHIFrameBufferDesc& FrameBufferStorageNode::GetFrameBufferDesc() const
{
	return FramebufferDesc;
}

void FrameBufferStorageNode::Update()
{
	//Not Needed?
}

void FrameBufferStorageNode::Resize()
{
	FBuffer->AutoResize();
}

void FrameBufferStorageNode::Create()
{
	FrameBuffer::AutoUpdateSize(FramebufferDesc);
	FBuffer = RHI::CreateFrameBuffer(DeviceObject, FramebufferDesc);
}

FrameBuffer * FrameBufferStorageNode::GetFramebuffer()
{
	return FBuffer;
}
