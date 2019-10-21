#include "RenderGraphProcessor.h"
#include "RenderGraph.h"
#include "RenderNode.h"
#include "NodeLink.h"
#include "StorageNode.h"
#include "StoreNodes\FrameBufferStorageNode.h"


RenderGraphProcessor::RenderGraphProcessor()
{}


RenderGraphProcessor::~RenderGraphProcessor()
{}

void RenderGraphProcessor::Process(RenderGraph * graph)
{
	BuildTransitions(graph);
}


void RenderGraphProcessor::BuildTransitions(RenderGraph* graph)
{
	RenderNode* Node = graph->RootNode;
	RenderNode* Lastnode = Node;
	Node = Node->GetNextNode();
	while (Node != nullptr)
	{
		if (Node->IsNodeActive())
		{
			for (int i = 0; i < Node->GetNumInputs(); i++)
			{
				NodeLink* output = Node->GetInput(i);
				if (output->TargetType != EStorageType::Framebuffer)
				{
					continue;
				}
				if (output->ResourceState == EResourceState::Undefined)
				{
					continue;
				}
				//#todo check node state valid for node
				if (Lastnode->GetNodeQueueType() != ENodeQueueType::Graphics)
				{
					continue;
				}
				NodeLink* TargetLink = output->StoreLink;
				if (TargetLink == nullptr)
				{
					//we are directly connected to a store node
					FrameBufferStorageNode* FB = StorageNode::NodeCast<FrameBufferStorageNode>(output->GetStoreTarget());
					if (FB != nullptr)
					{
						FB->InitalResourceState = output->ResourceState;
					}
				}
				else
				{
					ResourceTransition T;
					T.TargetState = output->ResourceState;
					T.Target = output;
					TargetLink->OwnerNode->AddEndTransition(T);
				}
			}
		}
		Lastnode = Node;
		Node = Node->GetNextNode();
	}
}

void RenderGraphProcessor::BuildAliasing(RenderGraph* graph)
{

}