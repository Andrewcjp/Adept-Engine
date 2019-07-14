#include "Stdafx.h"
#include "InterGPUStorageNode.h"
#include "RHI\RHIInterGPUStagingResource.h"


InterGPUStorageNode::InterGPUStorageNode()
{
	StoreType = EStorageType::InterGPUStagingResource;
}

InterGPUStorageNode::~InterGPUStorageNode()
{}

void InterGPUStorageNode::Update()
{}

void InterGPUStorageNode::Resize()
{}

int InterGPUStorageNode::ReserveSpaceForFB(FrameBuffer* FB)
{
	GPUStagingData D = GPUStagingData();

	D.Resource = RHI::GetRHIClass()->CreateInterGPUStagingResource(RHI::GetDefaultDevice(), InterGPUDesc());
	D.Resource->SizeforFramebuffer(FB);
	Resources.push_back(D);
	return Resources.size() - 1;
}

GPUStagingData* InterGPUStorageNode::GetStore(int index)
{
	return &Resources[index];
}

void InterGPUStorageNode::Create()
{

}
