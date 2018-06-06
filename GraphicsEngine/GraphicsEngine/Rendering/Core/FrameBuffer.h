#pragma once
#include "EngineGlobals.h"
#include "RHI/RHITypes.h"
class FrameBuffer
{
public:

	enum FrameBufferType { Depth, CubeDepth, Colour, ColourDepth,GBuffer ,Other};
	FrameBuffer(int width, int height, float ratio = 1, FrameBuffer::FrameBufferType type = FrameBufferType::ColourDepth);
	FrameBuffer(class DeviceContext* device, RHIFrameBufferDesc& Desc);
	virtual ~FrameBuffer();
	
	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }
	class DeviceContext* GetDevice();
	FrameBufferType GetType() const;
	const RHIFrameBufferDesc& GetDescription()const;
	virtual void Resize(int width, int height);

	//remove
	void InitBuffer(glm::vec4 clearColour);
	glm::vec4 GetClearColour() const;
	virtual void CreateCubeDepth();
	virtual void CreateColour(int Index = 0);
	virtual void CreateDepth();
	virtual void CreateGBuffer();
	int RenderTargetCount = 1;
	bool RequiresDepth();
	FrameBufferType m_ftype;
	glm::vec4 BufferClearColour = glm::vec4(1.0f);
	int m_width = 0;
	int m_height = 0;
	//end remove
protected:	
	DeviceContext* CurrentDevice = nullptr;
	RHIFrameBufferDesc BufferDesc;
	
};

