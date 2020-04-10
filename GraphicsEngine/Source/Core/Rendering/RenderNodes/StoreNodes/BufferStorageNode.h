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
protected:
	void Create() override;
	RHIBuffer* GPUBuffer;
};

