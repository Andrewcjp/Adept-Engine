
#include "InterGPUStorageNode.h"
#include "RHI\RHIInterGPUStagingResource.h"
#include "..\..\Core\FrameBuffer.h"
#include "FrameBufferStorageNode.h"
#include "BufferStorageNode.h"


InterGPUStorageNode::InterGPUStorageNode()
{
	StoreType = EStorageType::InterGPUStagingResource;
}

InterGPUStorageNode::~InterGPUStorageNode()
{
}

void InterGPUStorageNode::Update()
{
}

void InterGPUStorageNode::Resize()
{
	for (int i = 0; i < Resources.size(); i++)
	{
		SafeRHIRelease(Resources[i].Resource);
	}
	Resources.clear();
	for (int i = 0; i < StoreTargets.size(); i++)
	{
		ReserveSpaceForFB(StoreTargets[i]->GetFramebuffer());
	}
	for (int i = 0; i < BufferStoreTargets.size(); i++)
	{
		ReserveSpaceForBuffer(BufferStoreTargets[i]->GetBuffer());
	}
}

uint64 InterGPUStorageNode::ReserveSpaceForFB(FrameBuffer* FB)
{
	GPUStagingData D = GPUStagingData();
	InterGPUDesc desc;
	desc.FramebufferDesc = FB->GetDescription();
	D.Resource = RHI::GetRHIClass()->CreateInterGPUStagingResource(RHI::GetDefaultDevice(), desc);
	D.Resource->SizeforFramebuffer(FB);
	Resources.push_back(D);
	return Resources.size() - 1;
}
uint64 InterGPUStorageNode::ReserveSpaceForBuffer(RHIBuffer* FB)
{
	GPUStagingData D = GPUStagingData();
	InterGPUDesc desc;
	desc.IsBuffer = true;
	desc.BufferDesc = FB->GetDesc();
	D.Resource = RHI::GetRHIClass()->CreateInterGPUStagingResource(RHI::GetDefaultDevice(), desc);
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
