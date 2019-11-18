#pragma once
#include "StorageNode.h"
#include "RenderGraphDrawer.h"

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
	ConsoleVariable* CVar = nullptr;
	bool* TargetProp = nullptr;
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
	void ApplyEditorToGraph();
	void AddVRXSupport();
	void CreateDefGraphWithRT();
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



	void ListNodes();
	void ValidateGraph();
	void CreateVRFWDGraph();
	void CreatePathTracedGraph();
	void CreateFallbackGraph();

	void CreateMGPU_TESTGRAPH();

	std::vector<StorageNode*> GetNodesOfType(EStorageType::Type type);


	struct ValidateArgs
	{
		bool ErrorWrongFormat = false;
		void AddWarning(std::string Message);
		void AddError(std::string Message);
		bool HasError() const;
		std::vector<std::string> Warnings;
		std::vector<std::string> Errors;
		RenderGraph* TargetGraph = nullptr;
	private:
	};
	bool IsVRGraph = false;
	bool IsGraphValid();
	std::string GetGraphName() const { return GraphName; }
	void SetGraphName(std::string val) { GraphName = val; }

	RenderNode* FindFirstOf(const std::string& name);
	std::vector<RenderNode*> FindAllOf(const std::string& name);
	RenderNode* GetNodeAtIndex(int i);
	int GetIndexOfNode(RenderNode * Node);
	void RunTests();
	bool RequiresMGPU = false;
	bool RequiresRT = false;
private:
	std::vector<ConsoleVariable*> AutoVars;
	std::string GraphName = "";

	int NodeCount = 0;
	int ActiveNodeCount = 0;
	void ExposeItem(RenderNode* N, std::string name, bool Defaultstate = true);

	void ExposeNodeOption(RenderNode * N, std::string name, bool * data, bool Defaultstate);
	RenderGraphDrawer Drawer;
};

namespace StandardSettings
{
	const std::string UsePreZ = "PreZ";
	const std::string UseRaytrace = "RT";
	const std::string UseSSR = "SSR";
};