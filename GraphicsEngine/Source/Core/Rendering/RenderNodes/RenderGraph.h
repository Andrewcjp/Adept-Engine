#pragma once
#include "StorageNode.h"
#include "RenderGraphDrawer.h"
#include "RenderGraphProcessor.h"

class RenderNode;
class StorageNode;
class FrameBufferStorageNode;
class BranchNode;
class OutputToScreenNode;
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

struct RG_PatchMarker
{
	std::vector<NodeLink*> Inputs;
	std::vector<RenderNode*> OutputTargets;
	RenderNode* ExecuteIn = nullptr;
	RenderNode* ExecuteOut = nullptr;
	std::vector<StorageNode*> ExposedResources;
};
struct PatchBase
{
	RenderNode* ExecuteIn = nullptr;
	RenderNode* ExecuteOut = nullptr;
};

template<class T>
struct RG_PatchMark_NEW : public PatchBase
{
	T Data;
};


struct RG_PatchSet
{
	template<class T>
	const T* AccessMarker(int i)
	{
		return &((RG_PatchMark_NEW<T>*)Markers[i])->Data;
	}

	std::vector<PatchBase*> Markers;
	std::vector<EBuiltInRenderGraphPatch::Type> SupportedPatches;
	bool SupportsPatchType(EBuiltInRenderGraphPatch::Type type);
	void AddPatchMarker(PatchBase * patch, EBuiltInRenderGraphPatch::Type type);
};
struct RG_PatchMarkerCollection
{
	std::vector<RG_PatchSet*> Sets;
	void AddPatchSet(RG_PatchSet* patch);
	void AddSinglePatch(PatchBase* patch, EBuiltInRenderGraphPatch::Type type);
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
	void RefreshNodes();
	void Update();
	//calls the setup node on each node in execution order.
	void BuildGraph();
	void ApplyEditorToGraph();
	void AddVRXSupport();
	void CreateDefGraphWithRT();
	void CreateDefGraphWithRT_VOXEL();
	void CreateDefGraphWithVoxelRT();

	FrameBufferStorageNode* CreateRTXBuffer();

	void CreateDefTestgraph();

	void EndGraph(FrameBufferStorageNode* MainBuffer, RenderNode* Output);

	//BranchNode * AddBranchNode(RenderNode * Start, RenderNode * A, RenderNode * B, bool initalstate, std::string ExposeName = std::string());

	void LinkNode(RenderNode * A, RenderNode * B);

	void CreateFWDGraph();
	template<class T>
	T * AddStoreNode(T * Node)
	{
		StoreNodes.push_back(Node);
		Node->OwnerGraph = this;
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
	void CreateSFR();
	void CreateMGPUShadows();
	std::vector<StorageNode*> GetNodesOfType(EStorageType::Type type);

	RG_PatchSet * FindMarker(EBuiltInRenderGraphPatch::Type type);
	PatchBase * FindMarker2(EBuiltInRenderGraphPatch::Type type);

	struct ValidateArgs
	{
		bool ErrorWrongFormat = false;
		void AddWarning(std::string Message);
		void AddError(std::string Message);
		bool HasError() const;
		std::vector<std::pair<std::string, Log::Severity>> OutputLog;
		RenderGraph* TargetGraph = nullptr;
	private:
		bool Error = false;
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
	int TotalResourceSize = 0;
	void InvalidateGraph();
private:
	std::vector<ConsoleVariable*> AutoVars;
	std::string GraphName = "";
	bool GraphNeedsProcess = false;
	int NodeCount = 0;
	int ActiveNodeCount = 0;
	void ExposeItem(RenderNode* N, std::string name, bool Defaultstate = true);

	void ExposeNodeOption(RenderNode * N, std::string name, bool * data, bool Defaultstate);
	RenderGraphDrawer Drawer;
	RenderGraphProcessor Processor;
	RG_PatchMarkerCollection Markers;
};

namespace StandardSettings
{
	const std::string UsePreZ = "PreZ";
	const std::string UseRaytrace = "RT";
	const std::string UseSSR = "SSR";
};