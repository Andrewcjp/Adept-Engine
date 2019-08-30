#pragma once
#include "../StorageNode.h"

class FrameBuffer;
class FrameBufferStorageNode : public StorageNode
{
public:
	FrameBufferStorageNode();
	~FrameBufferStorageNode();
	void SetFrameBufferDesc(RHIFrameBufferDesc& desc);
	const RHIFrameBufferDesc& GetFrameBufferDesc() const;

	virtual void Update() override;
	virtual void Resize() override;
	virtual void Create() override;
	FrameBuffer* GetFramebuffer(EEye::Type eye = EEye::Left);
	bool IsVRFramebuffer= false;
protected:
	RHIFrameBufferDesc FramebufferDesc = {};
	FrameBuffer* FBuffer = nullptr;
	FrameBuffer* FRightEyeBuffer = nullptr;
};

