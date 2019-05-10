#pragma once

class FrameBuffer;
class RHICommandList;
//handles generating mips, blurring etc.
class FrameBufferProcessor
{
public:
	FrameBufferProcessor();
	~FrameBufferProcessor();
	static void GenerateBlurChain(FrameBuffer* buffer, RHICommandList* list);
	static void CreateMipChain(FrameBuffer* buffer, RHICommandList* list);
	static void BlurMipChain(FrameBuffer* buffer, RHICommandList* list);
private:
};

