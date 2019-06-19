#pragma once
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
}
class NodeLink;
class RenderNode
{
public:
	RenderNode();
	virtual ~RenderNode();
	void ExecuteNode();
	virtual void OnExecute() {};
	void LinkToNode(RenderNode* NextNode);
	RenderNode* Next = nullptr;
	RenderNode* LastNode = nullptr;
	virtual void SetupNode() {};
	NodeLink* GetInput(int index);
	NodeLink* GetOutput(int index);
	NodeLink* GetRefrence(int index);
	uint GetNumInputs()const;
	uint GetNumOutput()const;
	uint GetNumRefrences()const;
	EViewMode::Type GetViewMode()const;
	ENodeQueueType::Type GetNodeQueueType()const;
protected:
	ENodeQueueType::Type NodeEngineType = ENodeQueueType::Graphics;
	EViewMode::Type ViewMode = EViewMode::DontCare;
	std::vector<NodeLink*> Inputs;
	std::vector<NodeLink*> Outputs;
	std::vector<NodeLink*> Refrences;
};

