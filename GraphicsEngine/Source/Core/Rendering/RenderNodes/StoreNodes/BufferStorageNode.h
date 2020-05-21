#pragma once
#include "../StorageNode.h"
#include "RHI/RHICommandList.h"

class FrameBufferStorageNode;
class BufferStorageNode : public StorageNode
{
public:
	BufferStorageNode();
	virtual ~BufferStorageNode();
	void Update() override;
	void Resize() override;
	RHIBuffer* GetBuffer()const
	{
		return GPUBuffer;
	};
	RHIBufferDesc Desc = RHIBufferDesc();
	float LinkedFrameBufferRatio = 1.0f;
	FrameBufferStorageNode* FramebufferNode = nullptr;
	typedef std::function<void(RHIBufferDesc&, glm::ivec2)> BufferResizeFunc;
	BufferResizeFunc m_ResizeFunc;
protected:
	void Create() override;
	RHIBuffer* GPUBuffer;
};

