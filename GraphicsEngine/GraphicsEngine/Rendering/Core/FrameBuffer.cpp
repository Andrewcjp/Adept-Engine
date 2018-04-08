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

void FrameBuffer::InitBuffer()
{
	switch (m_ftype)
	{
	case ColourDepth:
		CreateColour();
	case Depth:
		CreateDepth();
		break;
	case CubeDepth:
		CreateCubeDepth();
		break;
	case GBuffer:
		CreateGBuffer();
		break;
	}
}

void FrameBuffer::CreateCubeDepth()
{}

void FrameBuffer::CreateColour()
{}

void FrameBuffer::CreateDepth()
{}

void FrameBuffer::CreateGBuffer()
{}

void FrameBuffer::Cleanup()
{}
