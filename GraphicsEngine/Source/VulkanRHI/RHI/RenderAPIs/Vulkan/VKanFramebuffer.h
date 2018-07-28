#pragma once
#include "Rendering/Core/FrameBuffer.h"
#if BUILD_VULKAN
class VKanFramebuffer : public FrameBuffer
{
public:
	VKanFramebuffer(int width, int height, float ratio = 1.0f, FrameBuffer::FrameBufferType type = FrameBufferType::ColourDepth);
	~VKanFramebuffer();

	// Inherited via FrameBuffer
	virtual void BindToTextureUnit(int unit = 0) override;
	virtual void BindBufferAsRenderTarget(ID3D12GraphicsCommandList * list = nullptr) override;
	virtual void UnBind() override;
	virtual void ClearBuffer(ID3D12GraphicsCommandList * list = nullptr) override;
};
#endif
