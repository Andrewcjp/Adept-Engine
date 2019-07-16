#pragma once

class RenderNode;
class StorageNode;
class FrameBufferStorageNode;
class BranchNode;
struct RenderGraphExposedSettings
{
	std::string name;
	void SetState(bool state);
	bool GetState()const;
	RenderGraphExposedSettings(RenderNode* Node, bool Default = true);
private:
	BranchNode* Branch = nullptr;
	RenderNode* ToggleNode = nullptr;
};
class RenderGraph
{
public:
	RenderGraph();
	~RenderGraph();

	void DestoryGraph();
	void RunGraph();
	void DebugOutput();
	void Resize();
	void ResetForFrame();
	void Update();
	//calls the setup node on each node in execution order.
	void BuildGraph();
	void CreateDefTestgraph();

	BranchNode * AddBranchNode(RenderNode * Start, RenderNode * A, RenderNode * B, bool initalstate, std::string ExposeName = std::string());

	void LinkNode(RenderNode * A, RenderNode * B);

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
	
	//#todo: this needs to tell systems like the textures etc to init textures on correct device(s)
	std::map<std::string, RenderGraphExposedSettings*> ExposedParms;
	void ToggleCondition(const std::string & name);
	bool SetCondition(std::string name, bool state);
	bool GetCondition(std::string name);

	
	void PrintNodeData();
	void ListNodes();
	void ValidateGraph();
	void CreateVRFWDGraph();
	void CreatePathTracedGraph();
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
	bool IsVRGraph = false;
private:
	std::string GraphName = "";
	int NodeCount = 0;
	int ActiveNodeCount = 0;
	void ExposeItem(RenderNode* N, std::string name, bool Defaultstate = true);
	
};

