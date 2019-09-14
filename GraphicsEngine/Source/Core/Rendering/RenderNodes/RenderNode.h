#pragma once
#include "StorageNode.h"
#include "RenderGraph.h"
namespace EViewMode
{
	enum Type
	{
		PerView,//node must run per eye in VR
		MultiView,//node can take a VR framebuffer input
		DontCare,//Things like Shadows that Don't care.
		Limit
	};
};

namespace ENodeQueueType
{
	enum Type
	{
		Graphics,
		Compute,
		RT,
		Copy,
		Limit
	};
};

class NodeLink;
class SceneDataNode;
class ShadowAtlasStorageNode;
class VRBranchNode;
class RenderNode
{
public:
	RenderNode();
	virtual ~RenderNode();
	//Called to update the inputs for this node etc.
	void UpdateSettings();

	//Implemented By every node to run It should be able to be invoked N times without issue
	virtual void OnExecute() = 0;


	//This is called when a setting is updated
	virtual void RefreshNode() {};

	//sets the next node to execute after this one.
	void LinkToNode(RenderNode* NextNode);
	void ExecuteNode();
	//Data Nodes
	NodeLink* GetInput(int index);
	NodeLink* GetOutput(int index);
	NodeLink* GetRefrence(int index);
	uint GetNumInputs() const;
	uint GetNumOutputs() const;
	uint GetNumRefrences() const;
	//Node Type
	//For VR Only
	EViewMode::Type GetViewMode() const;
	ENodeQueueType::Type GetNodeQueueType() const;
	RenderNode* GetNextNode()const;
	bool IsComputeNode()const;
	virtual std::string GetName()const;
	void ValidateNode(RenderGraph::ValidateArgs & args);
	void SetupNode();

	bool IsNodeDeferred() const;
	void SetNodeDeferredMode(bool val);
	bool IsNodeActive() const;
	void SetNodeActive(bool val);
	void FindVRContext();
	bool IsBranchNode()const;
	bool IsVRBranch() const;

	virtual bool IsNodeSupported(const RenderSettings& settings);
	void SetDevice(DeviceContext* D);
	NodeLink* GetInputLinkByName(const std::string& name);
	NodeLink* GetOutputLinkByName(const std::string& name);
	template<class T>
	static T* NodeCast(RenderNode* node)
	{
		return dynamic_cast<T*>(node);
	}
	EEye::Type GetEye();
protected:

	//search forwards until we reach the end VR node 

	//
	VRBranchNode* VRBranchContext = nullptr;
	bool IsVrBranchNode = false;
	bool NodeActive = true;

	//is this node configured for a deferred pipeline or a forward one
	//This is a special case as many nodes need the depth from the Gbuffer in deferred 
	//all other conditions should be handled with Node conditionals.
	bool IsNodeInDeferredMode = false;

	//helpers:
	FrameBuffer* GetFrameBufferFromInput(int index);
	ShadowAtlasStorageNode * GetShadowDataFromInput(int index);
	Scene* GetSceneDataFromInput(int index);
	virtual void OnNodeSettingChange();

	//Creates all data need to run a node on X device should NOT call functions on external objects (excluding Render systems like the MeshPipline).
	virtual void OnSetupNode() {};
	virtual void OnValidateNode(RenderGraph::ValidateArgs & args);
	void AddInput(EStorageType::Type TargetType, const std::string& format, const std::string& InputName = std::string());
	void AddOutput(EStorageType::Type TargetType, const std::string& format, const std::string& InputName = std::string());
	void AddOutput(NodeLink* Input, const std::string& format, const std::string& InputName = std::string());
	void AddRefrence(EStorageType::Type TargetType, const std::string& format, const std::string& InputName);
	void PassNodeThough(int inputindex, std::string newformat = std::string(), int outputinput = -1);
	RenderNode* Next = nullptr;
	RenderNode* LastNode = nullptr;
	ENodeQueueType::Type NodeEngineType = ENodeQueueType::Graphics;
	EViewMode::Type ViewMode = EViewMode::DontCare;
	std::vector<NodeLink*> Inputs;
	std::vector<NodeLink*> Outputs;
	std::vector<NodeLink*> Refrences;
	DeviceContext* Context = nullptr;
	//If true The node is responsible for invoking the next node.
	bool NodeControlsFlow = false;
};

#define NameNode(name) std::string GetName()const {return name;} static std::string GetNodeName(){return name;}