#include "NodeLink.h"

NodeLink::NodeLink()
{}

NodeLink::NodeLink(EStorageType::Type Type, std::string format)
{
	TargetType = Type;
	DataFormat = format;
}

NodeLink::~NodeLink()
{}

bool NodeLink::SetStore(StorageNode* target)
{
	if (TargetType != target->StoreType || DataFormat != target->DataFormat)
	{
		return false;
	}
	StoreTarget = target;
	return true;
}

bool NodeLink::IsValid()
{
	return StoreTarget != nullptr;
}

bool NodeLink::SetLink(NodeLink* link)
{
	if (ViewMode != link->ViewMode)
	{
		return false;
	}
	if (TargetType != link->TargetType || DataFormat != link->DataFormat)
	{
		return false;
	}
	StoreLink = link;
	return true;
}

StorageNode * NodeLink::GetStoreTarget() const
{
	if (StoreLink != nullptr)
	{
		return StoreLink->GetStoreTarget();
	}
	return StoreTarget;
}
