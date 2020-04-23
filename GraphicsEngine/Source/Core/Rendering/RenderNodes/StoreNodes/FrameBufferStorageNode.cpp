#include "FrameBufferStorageNode.h"
#include "Rendering/Core/FrameBuffer.h"
#include "../RenderGraph.h"

FrameBufferStorageNode::FrameBufferStorageNode(const std::string& name) :StorageNode(name)
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
	if (OwnerGraph != nullptr)
	{
		//OwnerGraph->TotalResourceSize += FBuffer->GetSizeOnGPU();
	}
	if (FRightEyeBuffer != nullptr)
	{
		FRightEyeBuffer->AutoResize();
	}
}

void FrameBufferStorageNode::Create()
{
	FrameBuffer::AutoUpdateSize(FramebufferDesc);
	FramebufferDesc.SimpleStartingState = InitalResourceState;
	//if (InitalResourceState == EResourceState::ComputeUse || )
	//{
	//	FramebufferDesc.StartingState = GPU_RESOURCE_STATES::RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	//}
	FBuffer = RHI::CreateFrameBuffer(GetDeviceObject(), FramebufferDesc);
	if (IsVRFramebuffer)
	{
		FRightEyeBuffer = RHI::CreateFrameBuffer(GetDeviceObject(), FramebufferDesc);
	}
}

FrameBuffer* FrameBufferStorageNode::GetFramebuffer(EEye::Type eye /*= EEye::Left*/)
{
	if (eye == EEye::Right)
	{
		return FRightEyeBuffer;
	}
	return FBuffer;
}
