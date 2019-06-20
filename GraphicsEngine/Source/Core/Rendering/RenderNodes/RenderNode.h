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
class RenderNode
{
public:
	RenderNode();
	virtual ~RenderNode();
	
	//Implemented By every node to run It should be able to be invoked N times without issue
	virtual void OnExecute() = 0;
	//Creates all data need to run a node on X device should NOT call functions on external objects (excluding Render systems like the MeshPipline).
	virtual void SetupNode() {};

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
	uint GetNumOutput() const;
	uint GetNumRefrences() const;
	//Node Type
	//For VR Only
	EViewMode::Type GetViewMode() const;
	ENodeQueueType::Type GetNodeQueueType() const;
	RenderNode* GetNextNode()const;
	virtual std::string GetName()const;
	void ValidateNode(RenderGraph::ValidateArgs & args);

protected:
	virtual void OnValidateNode(RenderGraph::ValidateArgs & args);
	void AddInput(EStorageType::Type TargetType, std::string format, std::string InputName = std::string());
	void AddOutput(EStorageType::Type TargetType, std::string format, std::string InputName = std::string());
	void AddOutput(NodeLink* Input, std::string format, std::string InputName = std::string());
	void AddRefrence(EStorageType::Type TargetType, std::string format, std::string InputName);
	RenderNode* Next = nullptr;
	RenderNode* LastNode = nullptr;
	ENodeQueueType::Type NodeEngineType = ENodeQueueType::Graphics;
	EViewMode::Type ViewMode = EViewMode::DontCare;
	std::vector<NodeLink*> Inputs;
	std::vector<NodeLink*> Outputs;
	std::vector<NodeLink*> Refrences;
};

