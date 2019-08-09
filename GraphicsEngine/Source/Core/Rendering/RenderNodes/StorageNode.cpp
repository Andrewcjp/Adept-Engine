#include "StorageNode.h"
#include "StorageNodeFormats.h"

StorageNode::StorageNode()
{
	DataFormat = StorageFormats::DefaultFormat;
}

StorageNode::~StorageNode()
{}

void StorageNode::SetFormat(const std::string & format)
{
	DataFormat = format;
}

void StorageNode::CreateNode()
{
	if (DeviceObject == nullptr)
	{
		DeviceObject = RHI::GetDefaultDevice();
	}
	Create();
	InitalFormat = DataFormat;
}

void StorageNode::Reset()
{
	DataFormat = InitalFormat;
}

void StorageNode::SetDevice(DeviceContext * D)
{
	DeviceObject = D;
}

