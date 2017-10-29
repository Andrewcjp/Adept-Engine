#include "FrameBuffer.h"



FrameBuffer::FrameBuffer(int width, int height, float ratio, FrameBuffer::FrameBufferType type)
{
	m_ftype = type;
	m_width = static_cast<int>(width*ratio);
	m_height = static_cast<int>(height*ratio);
//	this->CreateBuffer();
}
FrameBuffer::~FrameBuffer()
{
	Cleanup();
}

void FrameBuffer::Cleanup()
{

}
