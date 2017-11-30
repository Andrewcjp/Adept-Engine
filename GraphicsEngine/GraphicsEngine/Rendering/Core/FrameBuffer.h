#pragma once
#include <GLEW\GL\glew.h>
#include "OpenGL\OGLTexture.h"
#include <iostream>
#include "../EngineGlobals.h"
class FrameBuffer
{
public:

	enum FrameBufferType { Depth, CubeDepth, ColourDepth ,Other};
	FrameBuffer(int width, int height, float ratio = 1, FrameBuffer::FrameBufferType type = FrameBufferType::ColourDepth);
	
	
	virtual ~FrameBuffer();
	virtual void BindToTextureUnit(int unit = 0) = 0;
	virtual void BindBufferAsRenderTarget(CommandListDef* list = nullptr) = 0;
	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }
	virtual void UnBind() = 0;
	OGLTexture* GetRenderTexture() { return BufferTexture; }
	virtual void ClearBuffer(CommandListDef* list = nullptr) = 0;
protected:
	virtual void Cleanup();
	FrameBufferType m_ftype;
	OGLTexture* BufferTexture;
	int m_width = 0;
	int m_height = 0;
	GLuint frameBuffer;
	GLuint FrameRenderBuffer;
};

