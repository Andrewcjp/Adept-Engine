#pragma once
#include "../EngineGlobals.h"
class FrameBuffer
{
public:

	enum FrameBufferType { Depth, CubeDepth, ColourDepth,GBuffer ,Other};
	FrameBuffer(int width, int height, float ratio = 1, FrameBuffer::FrameBufferType type = FrameBufferType::ColourDepth);
	virtual ~FrameBuffer();
	void InitBuffer();
	virtual void BindToTextureUnit(int unit = 0) = 0;
	virtual void BindBufferAsRenderTarget(CommandListDef* list = nullptr) = 0;
	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }
	virtual void UnBind() = 0;
	virtual void ClearBuffer(CommandListDef* list = nullptr) = 0;
	virtual void CreateCubeDepth();
	virtual void CreateColour();
	virtual void CreateDepth();
	virtual void CreateGBuffer();
protected:
	virtual void Cleanup();
	FrameBufferType m_ftype;	
	int m_width = 0;
	int m_height = 0;
};

