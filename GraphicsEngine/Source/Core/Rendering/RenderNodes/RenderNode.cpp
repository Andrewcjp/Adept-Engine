#include "RenderNode.h"
#include "StorageNode.h"
#include "NodeLink.h"
#include "RenderGraph.h"

RenderNode::RenderNode()
{}

RenderNode::~RenderNode()
{}

void RenderNode::ExecuteNode()
{
	OnExecute();
	if (Next != nullptr)
	{
		Next->ExecuteNode();
	}
}

void RenderNode::LinkToNode(RenderNode * NextNode)
{
	Next = NextNode;
	NextNode->LastNode = this;
}

NodeLink * RenderNode::GetInput(int index)
{
	return Inputs[index];
}

NodeLink * RenderNode::GetOutput(int index)
{
	return Outputs[index];
}

NodeLink * RenderNode::GetRefrence(int index)
{
	return Refrences[index];
}

uint RenderNode::GetNumInputs() const
{
	return (uint)Inputs.size();
}

uint RenderNode::GetNumOutput() const
{
	return (uint)Outputs.size();
}

uint RenderNode::GetNumRefrences() const
{
	return (uint)Refrences.size();
}

EViewMode::Type RenderNode::GetViewMode() const
{
	return ViewMode;
}

ENodeQueueType::Type RenderNode::GetNodeQueueType() const
{
	return NodeEngineType;
}

RenderNode * RenderNode::GetNextNode() const
{
	return Next;
}

std::string RenderNode::GetName() const
{
	return "UNNAMED";
}

void RenderNode::ValidateNode(RenderGraph::ValidateArgs & args)
{
	for (NodeLink* NL : Inputs)
	{
		NL->Validate(args);
	}
	for (NodeLink* NL : Outputs)
	{
		NL->Validate(args);
	}
	for (NodeLink* NL : Refrences)
	{
		NL->Validate(args);
	}
}

void RenderNode::OnValidateNode(RenderGraph::ValidateArgs & args)
{

}

void RenderNode::AddInput(EStorageType::Type TargetType, std::string format, std::string InputName)
{
	Inputs.push_back(new NodeLink(TargetType, format, InputName));
}

void RenderNode::AddOutput(EStorageType::Type TargetType, std::string format, std::string InputName)
{
	Outputs.push_back(new NodeLink(TargetType, format, InputName));
}

void RenderNode::AddOutput(NodeLink * Input, std::string format, std::string InputName)
{
	AddOutput(Input->TargetType, format, InputName);
}

void RenderNode::AddRefrence(EStorageType::Type TargetType, std::string format, std::string InputName)
{
	Refrences.push_back(new NodeLink(TargetType, format, InputName));
}