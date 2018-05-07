#pragma once
#include "../EngineGlobals.h"
class FrameBuffer
{
public:

	enum FrameBufferType { Depth, CubeDepth, Colour, ColourDepth,GBuffer ,Other};
	FrameBuffer(int width, int height, float ratio = 1, FrameBuffer::FrameBufferType type = FrameBufferType::ColourDepth);
	virtual ~FrameBuffer();
	void InitBuffer(glm::vec4 clearColour);
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
	class DeviceContext* GetDevice();
	FrameBufferType GetType();
	glm::vec4 GetClearColour();
	virtual void Resize(int width, int height);
protected:
	virtual void Cleanup();
	FrameBufferType m_ftype;
	glm::vec4 BufferClearColour = glm::vec4(1.0f);
	int m_width = 0;
	int m_height = 0;
	DeviceContext* CurrentDevice = nullptr;
};

