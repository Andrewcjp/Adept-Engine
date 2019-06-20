#include "FrameBufferStorageNode.h"
#include "Rendering/Core/FrameBuffer.h"

FrameBufferStorageNode::FrameBufferStorageNode()
{}

FrameBufferStorageNode::~FrameBufferStorageNode()
{}

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
	FBuffer = RHI::CreateFrameBuffer(DeviceObject, FramebufferDesc);
}
