
#include "StorageNode.h"


StorageNode::StorageNode()
{}


StorageNode::~StorageNode()
{}

void StorageNode::CreateNode()
{
	if (DeviceObject == nullptr)
	{
		DeviceObject = RHI::GetDefaultDevice();
	}
	Create();
}

void StorageNode::SetDevice(DeviceContext * D)
{
	DeviceObject = D;
}

