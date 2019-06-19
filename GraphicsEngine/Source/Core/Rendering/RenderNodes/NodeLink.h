#pragma once
#include "StorageNode.h"
#include "RenderNode.h"

class RenderNode;
class StorageNode;

class NodeLink
{
public:
	NodeLink();
	NodeLink(EStorageType::Type TargetType, std::string format);
	~NodeLink();
	bool SetStore(StorageNode* Target);
	bool IsValid();
	bool SetLink(NodeLink* link);
	EStorageType::Type TargetType = EStorageType::Limit;
	RenderNode* Target = nullptr;
	StorageNode* GetStoreTarget() const;
	std::string DataFormat = "";
	//the link containing the data.
	NodeLink* StoreLink = nullptr;
	EViewMode::Type ViewMode = EViewMode::DontCare;
private:
	StorageNode* StoreTarget = nullptr;
};

