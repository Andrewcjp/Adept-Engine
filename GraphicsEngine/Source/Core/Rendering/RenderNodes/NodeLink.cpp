#include "NodeLink.h"
#include "StorageNodeFormats.h"

NodeLink::NodeLink()
{}

NodeLink::NodeLink(EStorageType::Type Type, std::string format, std::string InputName)
{
	TargetType = Type;
	DataFormat = format;
	SetLinkName(InputName);
}

NodeLink::~NodeLink()
{}

bool NodeLink::SetStore(StorageNode* target)
{
	if (TargetType != target->StoreType)
	{
		Log::LogMessage("Incorrect Type ");
		__debugbreak();
		return false;
	}
	if (RHI::GetFrameCount() == 0)
	{
		if (DataFormat != target->DataFormat)
		{
			if (DataFormat != StorageFormats::DefaultFormat)
			{
				Log::LogMessage("Incorrect Data Format");
			}
		}
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
	if (TargetType != link->TargetType)
	{
		return false;
	}
	if (DataFormat != link->DataFormat)
	{
		Log::LogMessage("Incorrect Data Format");
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

std::string NodeLink::GetLinkName() const
{
	return LinkName;
}

void NodeLink::SetLinkName(const std::string& val)
{
	LinkName = val;
}

void NodeLink::Validate(RenderGraph::ValidateArgs& args)
{
	if (StoreLink != nullptr && DataFormat != StoreLink->DataFormat)
	{
		if (args.ErrorWrongFormat)
		{
			args.AddError("NodeLink: " + GetLinkName() + " Incorrect Data Format");
		}
		else
		{
			args.AddWarning("NodeLink: " + GetLinkName() + " Incorrect Data Format");
		}
	}
}
