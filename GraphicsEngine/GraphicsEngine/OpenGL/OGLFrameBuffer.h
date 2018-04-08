#pragma once
#include "../Rendering/Core/FrameBuffer.h"
#if BUILD_OPENGL
class OGLFrameBuffer :
	public FrameBuffer
{
public:
	OGLFrameBuffer(int width, int height, float ratio = 1.0f, FrameBuffer::FrameBufferType type = FrameBufferType::ColourDepth) :FrameBuffer(width, height, ratio, type) {
		CreateBuffer();
	}
	virtual ~OGLFrameBuffer();

	void CreateBuffer();

	void Cleanup() override;

	void BindToTextureUnit(int unit) override;

	void BindBufferAsRenderTarget(CommandListDef * list = nullptr) override;

	void UnBind() override;


	// Inherited via FrameBuffer
	virtual void ClearBuffer(CommandListDef * list = nullptr) override;
	int Samples = 4;
	bool MultiSample = false;

};
#endif

