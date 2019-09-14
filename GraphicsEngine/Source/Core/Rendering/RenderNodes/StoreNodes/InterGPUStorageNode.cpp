
#include "InterGPUStorageNode.h"
#include "RHI\RHIInterGPUStagingResource.h"
#include "..\..\Core\FrameBuffer.h"
#include "FrameBufferStorageNode.h"


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

uint InterGPUStorageNode::ReserveSpaceForFB(FrameBuffer* FB)
{
	GPUStagingData D = GPUStagingData();
	InterGPUDesc desc;
	desc.FramebufferDesc = FB->GetDescription();
	D.Resource = RHI::GetRHIClass()->CreateInterGPUStagingResource(RHI::GetDefaultDevice(), desc);
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
	for (int i = 0; i < StoreTargets.size(); i++)
	{
		ReserveSpaceForFB(StoreTargets[i]->GetFramebuffer());
	}
}
