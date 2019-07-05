#pragma once

class RenderNode;
class StorageNode;
class FrameBufferStorageNode;
class BranchNode;
class RenderGraph
{
public:
	RenderGraph();
	~RenderGraph();

	void DestoryGraph();
	void RunGraph();
	void Resize();
	void ResetForFrame();
	void Update();
	//calls the setup node on each node in execution order.
	void BuildGraph();
	void CreateDefTestgraph();

	BranchNode * AddBranchNode(RenderNode * Start, RenderNode * A, RenderNode * B, bool initalstate);

	void CreateFWDGraph();
	template<class T>
	T * AddStoreNode(T * Node)
	{
		StoreNodes.push_back(Node);
		return (T*)Node;
	}

	//All nodes run in series
	RenderNode* RootNode = nullptr;
	std::vector<StorageNode*> StoreNodes;
	RenderNode* OptionNode = nullptr;
	//#todo: this needs to tell systems like the textures etc to init textures on correct device(s)
	std::vector<BranchNode*> BranchNodes;

	void SetCondition(int nodeIndex, bool state);
	bool GetCondition(int nodeIndex);
	void PrintNodeData();
	void ValidateGraph();
	struct ValidateArgs
	{
		bool ErrorWrongFormat = false;
		void AddWarning(std::string Message);
		void AddError(std::string Message);
		bool HasError() const;
		std::vector<std::string> Warnings;
		std::vector<std::string> Errors;
	private:

	};
};

