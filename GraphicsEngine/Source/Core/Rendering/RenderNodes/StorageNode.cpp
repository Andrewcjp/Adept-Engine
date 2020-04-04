#include "StorageNode.h"
#include "StorageNodeFormats.h"

StorageNode::StorageNode(const std::string& name)
	:DataFormat(StorageFormats::DefaultFormat),
	Name(name)
{}

StorageNode::~StorageNode()
{}

void StorageNode::SetFormat(const std::string & format)
{
	DataFormat = format;
}

void StorageNode::CreateNode()
{
	if (GetDeviceObject() == nullptr)
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

void StorageNode::DiscardAtEndOfNode(RenderNode* node)
{
	DiscardingNode = node;
}

