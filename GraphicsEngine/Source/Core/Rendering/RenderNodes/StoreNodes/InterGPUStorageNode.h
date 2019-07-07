#pragma once
#include "../StorageNode.h"
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
	int ReserveSpaceForFB(FrameBuffer* FB);
	GPUStagingData* GetStore(int index);
protected:
	virtual void Create() override;
	
	std::vector<GPUStagingData> Resources;

};

