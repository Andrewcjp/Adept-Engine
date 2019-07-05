#pragma once
#include "../StorageNode.h"

class RHICommandList;
class ShadowRenderer;
//special node for shadow data as it is multiple framebuffer or atlas etc.
class ShadowAtlasStorageNode : public StorageNode
{
public:
	ShadowAtlasStorageNode();
	~ShadowAtlasStorageNode();

	virtual void Update() override;
	virtual void Resize() override;

	void BindDirectionArray(RHICommandList* List, int Slot);
	void BindPointArray(RHICommandList* List, int Slot);
	ShadowRenderer* Shadower = nullptr;
protected:
	virtual void Create() override;
	
};

