#pragma once
#include "../StorageNode.h"

class FrameBufferStorageNode;
//the Render graph will need to reserve space which will be allocated.
//each copy will be tracked by this node and validated to avoid wrong data etc.
struct GPUStagingData
{
	RHIInterGPUStagingResource* Resource;
	bool IsInUse = false;
};
class InterGPUStorageNode: public StorageNode
{
public:
	InterGPUStorageNode();
	~InterGPUStorageNode();

	virtual void Update() override;
	virtual void Resize() override;
	
	GPUStagingData* GetStore(int index);
	std::vector<FrameBufferStorageNode*> StoreTargets;
protected:
	virtual void Create() override;
	int ReserveSpaceForFB(FrameBuffer* FB);
	std::vector<GPUStagingData> Resources;

};

