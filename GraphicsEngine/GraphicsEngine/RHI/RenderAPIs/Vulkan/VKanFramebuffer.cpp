#include "stdafx.h"
#include "VKanFramebuffer.h"
#if BUILD_VULKAN



VKanFramebuffer::VKanFramebuffer(int width, int height, float ratio, FrameBuffer::FrameBufferType type) :FrameBuffer(width, height, ratio, type)
{
	//CreateBuffer();
}

VKanFramebuffer::~VKanFramebuffer()
{}

void VKanFramebuffer::BindToTextureUnit(int unit)
{}

void VKanFramebuffer::BindBufferAsRenderTarget(ID3D12GraphicsCommandList * list)
{}

void VKanFramebuffer::UnBind()
{}

void VKanFramebuffer::ClearBuffer(ID3D12GraphicsCommandList * list)
{}
#endif