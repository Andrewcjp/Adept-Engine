#include "BufferStorageNode.h"
#include "FrameBufferStorageNode.h"
#include "RHI/SFRController.h"
#include "../../Core/Screen.h"

BufferStorageNode::BufferStorageNode()
{
	StoreType = EStorageType::Buffer;
	Desc.StartState = EResourceState::Common;
}

BufferStorageNode::~BufferStorageNode()
{}

void BufferStorageNode::Update()
{

}

void BufferStorageNode::Resize()
{
	EnqueueSafeRHIRelease(GPUBuffer);
	Create();
}

void BufferStorageNode::Create()
{
	if (FramebufferNode != nullptr)
	{
		int Size = FramebufferNode->GetFrameBufferDesc().Width * FramebufferNode->GetFrameBufferDesc().Height;
		int compCount = RHIUtils::GetComponentCount(FramebufferNode->GetFrameBufferDesc().RTFormats[0]);
		if (RHI::GetRenderSettings()->GetCurrnetSFRSettings().Enabled)
		{
			RHIScissorRect r = SFRController::GetScissor(1, Screen::GetScaledRes());
			int Width = abs(r.Right - r.Left);
			Size = Width * r.Bottom;
			compCount = 3;
		}
		else
		{
			compCount = 1;
			LinkedFrameBufferRatio = 1.0 / 8.0f;
			//Size = FramebufferNode->GetFrameBufferDesc().Width/8 * FramebufferNode->GetFrameBufferDesc().Height;
		}
		Desc.ElementCount = Math::Max(1, int((Size * compCount) * LinkedFrameBufferRatio));
		Desc.Stride = RHI::GetRenderSettings()->GetCurrnetSFRSettings().Use8BitCompression ? 1 : 2;
	}
	Desc.AllowUnorderedAccess = true;
	Desc.Accesstype = EBufferAccessType::GPUOnly;
	if (m_ResizeFunc != nullptr)
	{
		m_ResizeFunc(Desc, Screen::GetScaledRes());
	}
	GPUBuffer = RHI::CreateRHIBuffer(ERHIBufferType::GPU, GetDeviceObject());
	GPUBuffer->CreateBuffer(Desc);
}
