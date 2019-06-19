#include "RenderNode.h"

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
	return Inputs.size();
}

uint RenderNode::GetNumOutput() const
{
	return Outputs.size();
}

uint RenderNode::GetNumRefrences() const
{
	return Refrences.size();
}

EViewMode::Type RenderNode::GetViewMode() const
{
	return ViewMode;
}

ENodeQueueType::Type RenderNode::GetNodeQueueType() const
{
	return NodeEngineType;
}
