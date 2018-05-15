#include "FrameBuffer.h"



FrameBuffer::FrameBuffer(int width, int height, float ratio, FrameBuffer::FrameBufferType type)
{
	m_ftype = type;
	m_width = static_cast<int>(width*ratio);
	m_height = static_cast<int>(height*ratio);

}
FrameBuffer::~FrameBuffer()
{
	Cleanup();
}

void FrameBuffer::InitBuffer(glm::vec4 clearColour)
{
	BufferClearColour = clearColour;
	switch (m_ftype)
	{
	case ColourDepth:
		RenderTargetCount = 1;
		CreateColour();
		CreateDepth();
		break;
	case Depth:
		RenderTargetCount = 0;
		CreateDepth();
		break;
	case CubeDepth:
		RenderTargetCount = 1;
		CreateCubeDepth();
		break;
	case GBuffer:
		RenderTargetCount = 4;
		CreateGBuffer();
		break;
	}
}

void FrameBuffer::CreateCubeDepth()
{}

void FrameBuffer::CreateColour(int Index)
{}

void FrameBuffer::CreateDepth()
{}

void FrameBuffer::CreateGBuffer()
{}

DeviceContext * FrameBuffer::GetDevice()
{
	return CurrentDevice;
}

FrameBuffer::FrameBufferType FrameBuffer::GetType() const 
{
	return m_ftype;
}

glm::vec4 FrameBuffer::GetClearColour() const
{
	return BufferClearColour;
}

void FrameBuffer::Resize(int width, int height)
{}

bool FrameBuffer::RequiresDepth()
{
	return m_ftype == FrameBufferType::Depth || m_ftype == FrameBufferType::ColourDepth || m_ftype == FrameBufferType::GBuffer;
}

void FrameBuffer::Cleanup()
{}
