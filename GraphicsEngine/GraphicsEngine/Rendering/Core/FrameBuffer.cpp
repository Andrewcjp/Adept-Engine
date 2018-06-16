#include "FrameBuffer.h"

FrameBuffer::FrameBuffer(DeviceContext * device, RHIFrameBufferDesc & Desc)
{
	CurrentDevice = device;
	BufferDesc = Desc;
	m_width = BufferDesc.Width;
	m_height = BufferDesc.Height;
}

FrameBuffer::~FrameBuffer()
{
}

DeviceContext * FrameBuffer::GetDevice()
{
	return CurrentDevice;
}

const RHIFrameBufferDesc & FrameBuffer::GetDescription() const
{
	return BufferDesc;
}

void FrameBuffer::Resize(int width, int height)
{}

