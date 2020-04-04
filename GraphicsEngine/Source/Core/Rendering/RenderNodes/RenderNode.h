#pragma once
#include "StorageNode.h"
#include "RenderGraph.h"
#define InputData(name,x) struct name {x }; name NodeInputStruct; name & GetData(){return NodeInputStruct;}
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

class NodeLink;
class SceneDataNode;
class ShadowAtlasStorageNode;
class VRBranchNode;
class RenderNode;
class Scene;
struct RHIBufferDesc;
class RHICommandList;

struct ResourceTransition
{
	enum Type
	{
		StateChange,
		QueueWait,
		AlisingBarrier,
		Limit
	};
	ResourceTransition::Type TransitonType = Limit;
	EResourceTransitionMode::Type TransitionMode = EResourceTransitionMode::Direct;
	NodeLink* Target = nullptr;
	StorageNode* StoreNode = nullptr;
	EResourceState::Type TargetState = EResourceState::Undefined;
	void Execute(RHICommandList* list, RenderNode* eye);

	//queue wait
	DeviceContextQueue::Type SignalingQueue = DeviceContextQueue::LIMIT;
	int SignalingDevice = -1;
};
struct ENodeWorkFocus 
{
	enum Type 
	{
		Compute,
		Memory,
		Graphics,
		Raytrace,
		All,
		Limit,
	};
};
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
	uint GetNumInputs() const;
	uint GetNumOutputs() const;
	//Node Type
	//For VR Only
	EViewMode::Type GetViewMode() const;
	ECommandListType::Type GetNodeQueueType() const;
	DeviceContextQueue::Type GetNodeQueue()const;
	RenderNode* GetNextNode()const;
	bool IsComputeNode()const;
	virtual std::string GetName()const;
	void ValidateNode(RenderGraph::ValidateArgs& args);
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
	void AddBeginTransition(const ResourceTransition& transition);
	void AddEndTransition(const ResourceTransition& transition);
	virtual void OnResourceResize();
	RenderNode* GetLastNode() const { return LastNode; }
	void SetLastNode(RenderNode* val) { LastNode = val; }
	int GetDeviceIndex()const;
	virtual void OnGraphCreate();
	void SetTargetEye(EEye::Type eye);
	
	bool GetUseSeperateCommandList() const { return UseSeperateCommandList; }
	ENodeWorkFocus::Type GetWorkType() const { return WorkType; }
protected:
	void SetWorkType(ENodeWorkFocus::Type val) { WorkType = val; }
	void SetUseSeperateCommandList() { UseSeperateCommandList = true; }

	//search forwards until we reach the end VR node 

	//
	VRBranchNode* VRBranchContext = nullptr;
	bool IsVrBranchNode = false;
	bool NodeActive = true;
	EEye::Type TargetEye = EEye::Left;
	//is this node configured for a deferred pipeline or a forward one
	//This is a special case as many nodes need the depth from the Gbuffer in deferred 
	//all other conditions should be handled with Node conditionals.
	bool IsNodeInDeferredMode = false;

	//helpers:
	FrameBuffer* GetFrameBufferFromInput(int index);
	FrameBuffer* GetFrameBufferFromInput(NodeLink* link);
	RHIBuffer* GetBufferFromInput(int index);
	ShadowAtlasStorageNode* GetShadowDataFromInput(int index);
	ShadowAtlasStorageNode* GetShadowDataFromInput(NodeLink* link);
	Scene* GetSceneDataFromInput(int index);
	virtual void OnNodeSettingChange();

	//Creates all data need to run a node on X device should NOT call functions on external objects (excluding Render systems like the MeshPipline).
	virtual void OnSetupNode() {};
	virtual void OnValidateNode(RenderGraph::ValidateArgs& args);
	NodeLink* AddResourceInput(EStorageType::Type Type, EResourceState::Type State, const std::string& format, const std::string& InputName = std::string());
	void AddResourceOutput(EStorageType::Type TargetType, EResourceState::Type State, const std::string& format, const std::string& InputName = std::string());
	NodeLink* AddInput(EStorageType::Type TargetType, const std::string& format, const std::string& InputName = std::string());
	void AddOutput(EStorageType::Type TargetType, const std::string& format, const std::string& InputName = std::string());
	void AddOutput(NodeLink* Input, const std::string& format, const std::string& InputName = std::string());
	StorageNode* RequestBuffer(const RHIBufferDesc& desc, std::string Name = "", FrameBufferStorageNode* LinkedNode = nullptr);
	StorageNode* RequestFrameBuffer(const RHIFrameBufferDesc& desc, std::string Name = "");
	NodeLink* AddFrameBufferResource(EResourceState::Type State, const RHIFrameBufferDesc& desc, const std::string& InputName = std::string());
	NodeLink* AddBufferResource(EResourceState::Type State, const RHIBufferDesc& desc, const std::string& InputName = std::string(),FrameBufferStorageNode* LinkedBuffer = nullptr);
	RHICommandList* GetList();
	RHICommandList* GetListAndReset();
	void ExecuteList(bool Flush = false);
	void LinkThough(int inputindex, int outputindex = -1);
	void PassNodeThough(int inputindex, std::string newformat = std::string(), int outputinput = -1);
	RenderNode* Next = nullptr;
	RenderNode* LastNode = nullptr;
	ECommandListType::Type NodeEngineType = ECommandListType::Graphics;
	DeviceContextQueue::Type NodeQueueType = DeviceContextQueue::Graphics;
	EViewMode::Type ViewMode = EViewMode::DontCare;
	bool AllowAsyncCompute = false;
	std::vector<NodeLink*> Inputs;
	std::vector<NodeLink*> Outputs;

	DeviceContext* Context = nullptr;
	//If true The node is responsible for invoking the next node.
	bool NodeControlsFlow = false;
	void SetBeginStates(RHICommandList* list);
	void SetEndStates(RHICommandList* list);

	std::vector<ResourceTransition> BeginTransitions;
	std::vector<ResourceTransition> EndTransitions;
	bool HasRunBegin = false;
	bool HasRunEnd = false;
	bool UseSeperateCommandList = false;
	RHICommandList* CommandList = nullptr;
	ENodeWorkFocus::Type WorkType = ENodeWorkFocus::All;
};

#define NameNode(name) std::string GetName()const {return name;} static std::string GetNodeName(){return name;}