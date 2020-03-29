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
	int LinkedFrameBufferRatio = 1;
	FrameBufferStorageNode* FramebufferNode = nullptr;
protected:
	void Create() override;
	RHIBuffer* GPUBuffer;
};

