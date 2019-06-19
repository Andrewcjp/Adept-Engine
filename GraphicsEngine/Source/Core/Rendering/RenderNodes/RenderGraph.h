#pragma once

class RenderNode;
class StorageNode;
class RenderGraph
{
public:
	RenderGraph();
	~RenderGraph();

	void RunGraph();

	void CreateTestgraph();

	StorageNode * AddStoreNode(StorageNode * Node);

	//All nodes run in series
	RenderNode* RootNode = nullptr;
	std::vector<StorageNode*> StoreNodes;
};

