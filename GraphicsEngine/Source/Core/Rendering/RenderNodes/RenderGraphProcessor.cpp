#include "RenderGraphProcessor.h"
#include "RenderGraph.h"
#include "RenderNode.h"
#include "NodeLink.h"
#include "StorageNode.h"
#include "StoreNodes\FrameBufferStorageNode.h"
#include "StoreNodes\BufferStorageNode.h"
#include "Core\Performance\PerfManager.h"


RenderGraphProcessor::RenderGraphProcessor()
{}


RenderGraphProcessor::~RenderGraphProcessor()
{}

void RenderGraphProcessor::Process(RenderGraph * graph)
{
	{
		SCOPE_STARTUP_COUNTER("RG Process");
		BuildTimeLine(graph);
		BuildTransitions(graph);
		//BuildTransitionsSplit(graph);
		BuildScheduling(graph);
		BuildAliasing(graph);
	}
	PerfManager::Get()->FlushSingleActionTimer("RG Process", true);
}

void RenderGraphProcessor::Reset()
{
	MemoryUtils::DeleteVector(TimeLines);
}

void RenderGraphProcessor::BuildTimeLine(RenderGraph* graph)
{
	RenderNode* Node = graph->RootNode;
	RenderNode* Lastnode = Node;
	//Node = Node->GetNextNode();
	while (Node != nullptr)
	{
		if (Node->IsNodeActive())
		{
			bool DidAddFrame = true;
			for (int i = 0; i < Node->GetNumInputs(); i++)
			{
				NodeLink* output = Node->GetInput(i);
				if (output->TargetType != EStorageType::Framebuffer && output->TargetType != EStorageType::InterGPUStagingResource && output->TargetType != EStorageType::Buffer)
				{
					continue;
				}
				if (output->ResourceState == EResourceState::Undefined && output->TargetType == EStorageType::Framebuffer)
				{
					continue;
				}
				if (output->GetStoreTarget() == nullptr)
				{
					Log::LogMessage("Link '" + output->GetLinkName() + "' from node '" + output->OwnerNode->GetName() + "' does not reach a resource ptr", Log::Error);
					//DEBUGBREAK;
					continue;
				}
				ResourceTimeLine* timeline = GetOrCreateTimeLine(output->GetStoreTarget());
				ResourceTimelineFrame* frame = new ResourceTimelineFrame();
				frame->Node = Node;
				frame->State = output->ResourceState;
				frame->TargetLink = output;
				timeline->Frames.push_back(frame);
				DidAddFrame = true;
			}
		}
		Node = Node->GetNextNode();
	}
}
struct ResourceRange
{
	int StartIndex = 0;
	int EndIndex = 0;
	ResourceTimeLine* TimeLine = nullptr;
};
void RenderGraphProcessor::BuildAliasing(RenderGraph* graph)
{
	std::vector< ResourceRange*> Ranges;
	for (int line = 0; line < TimeLines.size(); line++)
	{
		ResourceTimeLine* timeline = TimeLines[line];
		if (!timeline->Resource->GetIsTransisent())
		{
			continue;
		}
		ResourceRange* range = new ResourceRange();
		range->StartIndex = graph->GetIndexOfNode(timeline->Frames[0]->Node);
		range->EndIndex = graph->GetIndexOfNode(timeline->Frames[timeline->Frames.size() - 1]->Node);
		range->TimeLine = timeline;
		Ranges.push_back(range);
	}
	std::vector<std::vector<ResourceRange*>> ExclusiveRanges;
	ExclusiveRanges.resize(Ranges.size());
	for (int i = 0; i < Ranges.size(); i++)
	{
		for (int y = 0; y < Ranges.size(); y++)
		{
			int startindex = Ranges[i]->StartIndex;
			if (startindex >= Ranges[y]->StartIndex && startindex <= Ranges[y]->EndIndex)
			{
				continue;
			}
			int endindex = Ranges[i]->EndIndex;
			if (endindex >= Ranges[y]->StartIndex && endindex <= Ranges[y]->EndIndex)
			{
				continue;
			}
			ExclusiveRanges[y].push_back(Ranges[i]);
		}
	}
	//todo: aliasing determination 
}
void RenderGraphProcessor::BuildScheduling(RenderGraph * graph)
{
	int count = 0;
	for (int line = 0; line < TimeLines.size(); line++)
	{
		ResourceTimeLine* timeline = TimeLines[line];
		for (int i = 0; i < timeline->Frames.size(); i++)
		{
			if (i == 0)
			{
				//continue;
			}
			ResourceTimelineFrame* frame = timeline->Frames[i];
			int LastNodeIndex = (i - 1 < 0 ? timeline->Frames.size() - 1 : i - 1);
			ResourceTimelineFrame* Lastframe = timeline->Frames[LastNodeIndex];
			bool DeviceMatch = frame->Node->GetDeviceIndex() == Lastframe->Node->GetDeviceIndex();
			bool QueueMatch = frame->Node->GetNodeQueueType() == Lastframe->Node->GetNodeQueueType();
			if (DeviceMatch && QueueMatch)
			{
				continue;
			}
			ResourceTransition T;
			T.TransitonType = ResourceTransition::QueueWait;
			T.SignalingQueue = Lastframe->Node->GetNodeQueue();
			if (DeviceMatch)
			{
				T.SignalingDevice = -1;
				frame->Node->AddBeginTransition(T);
			}
			else
			{

				if (i == 0)
				{
					T.SignalingDevice = frame->Node->GetDeviceIndex();
					Lastframe->Node->AddBeginTransition(T);
				}
				else
				{
					T.SignalingDevice = Lastframe->Node->GetDeviceIndex();
					frame->Node->AddBeginTransition(T);
				}
			}
			//add unique

			count++;
		}
	}
	Log::LogMessage("Scheduling syncs count: " + std::to_string(count));
}

void RenderGraphProcessor::BuildScheduling_old(RenderGraph * graph)
{
	int count = 0;
	for (int line = 0; line < TimeLines.size(); line++)
	{
		ResourceTimeLine* timeline = TimeLines[line];
		for (int i = 0; i < timeline->Frames.size(); i++)
		{
			if (i == 0)
			{
				continue;
			}
			ResourceTimelineFrame* frame = timeline->Frames[i];
			ResourceTimelineFrame* Lastframe = timeline->Frames[i - 1];
			if (frame->Node->GetNodeQueueType() == Lastframe->Node->GetNodeQueueType())
			{
				continue;
			}
			ResourceTransition T;
			T.TransitonType = ResourceTransition::QueueWait;
			T.SignalingQueue = DeviceContextQueue::GetFromCommandListType(Lastframe->Node->GetNodeQueueType());
			//add unique
			frame->Node->AddBeginTransition(T);
			count++;
		}
	}
	Log::LogMessage("Scheduling syncs count: " + std::to_string(count));
}

void RenderGraphProcessor::BuildTransitionsSplit(RenderGraph* graph)
{
	const bool LogTranstions = true;
	int count = 0;
	for (int line = 0; line < TimeLines.size(); line++)
	{
		ResourceTimeLine* timeline = TimeLines[line];
		if (timeline->Resource->StoreType == EStorageType::InterGPUStagingResource)
		{
			continue;
		}
		for (int i = 0; i < timeline->Frames.size(); i++)
		{
			ResourceTimelineFrame* frame = timeline->Frames[i];
			if (i == 0)
			{
				StorageNode* store = timeline->Resource;
				if (store == nullptr)
				{
					store = frame->TargetLink->GetStoreTarget();
				}
				FrameBufferStorageNode* FB = StorageNode::NodeCast<FrameBufferStorageNode>(store);
				if (FB != nullptr)
				{
					FB->InitalResourceState = frame->State;
				}
				continue;
			}
			//todo: handle compute list transitions 
			ResourceTransition T;
			T.TransitonType = ResourceTransition::StateChange;
			T.TargetState = frame->State;
			T.Target = frame->TargetLink;
			RenderNode* Targetnode = timeline->Frames[i - 1]->Node;
			bool CanNodeTransition = EResourceState::IsStateValidForList(Targetnode->GetNodeQueueType(), frame->State);
			if (!CanNodeTransition)
			{
				//ensure(EResourceState::IsStateValidForList(frame->Node->GetNodeQueueType(), frame->State));
				frame->Node->AddBeginTransition(T);
				if (LogTranstions)
				{
					std::string data = "Node " + frame->Node->GetName() + " Direct Transitions resource " + timeline->Resource->Name + " to state: "
						+ EResourceState::ToString(frame->State);
					Log::LogMessage(data);
				}
			}
			else
			{
				T.TransitionMode = EResourceTransitionMode::Start;
				Targetnode->AddEndTransition(T);
				T.TransitionMode = EResourceTransitionMode::End;
				frame->Node->AddBeginTransition(T);
				if (LogTranstions)
				{
					std::string data = " Node " + Targetnode->GetName() + " Transitions resource " + timeline->Resource->Name + " to state: "
						+ EResourceState::ToString(frame->State);
					Log::LogMessage(data);
				}
			}
			if (i == timeline->Frames.size() - 1)
			{
				T.TargetState = timeline->Frames[0]->State;
				T.Target = timeline->Frames[0]->TargetLink;
				T.TransitionMode = EResourceTransitionMode::Direct;
				//add a transition for the next graph run
				frame->Node->AddEndTransition(T);
			}
			count++;
		}
	}
	Log::LogMessage("Resource Transitions count: " + std::to_string(count));
}

void RenderGraphProcessor::BuildTransitions(RenderGraph* graph)
{
	const bool LogTranstions = true;
	int count = 0;
	for (int line = 0; line < TimeLines.size(); line++)
	{
		ResourceTimeLine* timeline = TimeLines[line];
		if (timeline->Resource->StoreType == EStorageType::InterGPUStagingResource)
		{
			continue;
		}
		for (int i = 0; i < timeline->Frames.size(); i++)
		{
			ResourceTimelineFrame* frame = timeline->Frames[i];
			if (i == 0)
			{
				StorageNode* store = timeline->Resource;
				if (store == nullptr)
				{
					store = frame->TargetLink->GetStoreTarget();
				}
				FrameBufferStorageNode* FB = StorageNode::NodeCast<FrameBufferStorageNode>(store);
				if (FB != nullptr)
				{
					FB->InitalResourceState = frame->State;
				}
				BufferStorageNode* Buffer = StorageNode::NodeCast<BufferStorageNode>(store);
				if (Buffer != nullptr)
				{
					Buffer->Desc.StartState = frame->State;
				}
				continue;
			}
			//todo: handle compute list transitions 
			ResourceTransition T;
			T.TransitonType = ResourceTransition::StateChange;
			T.TargetState = frame->State;
			T.Target = frame->TargetLink;
			RenderNode* Targetnode = timeline->Frames[i - 1]->Node;
			bool CanNodeTransition = EResourceState::IsStateValidForList(Targetnode->GetNodeQueueType(), frame->State);
			if (!CanNodeTransition)
			{
				ensure(EResourceState::IsStateValidForList(frame->Node->GetNodeQueueType(), frame->State));
				frame->Node->AddBeginTransition(T);
				if (LogTranstions)
				{
					std::string data = " Node " + frame->Node->GetName() + " Transitions resource " + timeline->Resource->Name + " to state: "
						+ EResourceState::ToString(frame->State);
					Log::LogMessage(data);
				}
			}
			else
			{
				Targetnode->AddEndTransition(T);
				if (LogTranstions)
				{
					std::string data = " Node " + Targetnode->GetName() + " Transitions resource " + timeline->Resource->Name + " to state: "
						+ EResourceState::ToString(frame->State);
					Log::LogMessage(data);
				}
			}
			if (i == timeline->Frames.size() - 1)
			{
				T.TargetState = timeline->Frames[0]->State;
				T.Target = timeline->Frames[0]->TargetLink;
				T.TransitionMode = EResourceTransitionMode::Direct;

				//add a transition for the next graph run
				if (EResourceState::IsStateValidForList(frame->Node->GetNodeQueueType(), T.TargetState))
				{
					frame->Node->AddEndTransition(T);
					if (LogTranstions)
					{
						std::string data = " Node " + frame->Node->GetName() + " Transitions resource " + timeline->Resource->Name + " to state: "
							+ EResourceState::ToString(T.TargetState);
						Log::LogMessage(data);
					}
				}
				else
				{
					ensure(EResourceState::IsStateValidForList(timeline->Frames[0]->Node->GetNodeQueueType(), T.TargetState));
					timeline->Frames[0]->Node->AddBeginTransition(T);
					if (LogTranstions)
					{
						std::string data = " Node " + timeline->Frames[0]->Node->GetName() + " Transitions resource " + timeline->Resource->Name + " to state: "
							+ EResourceState::ToString(T.TargetState);
						Log::LogMessage(data);
					}
				}
			}
			count++;
		}
	}
	Log::LogMessage("Resource Transitions count: " + std::to_string(count));
}
ResourceTimeLine * RenderGraphProcessor::GetOrCreateTimeLine(StorageNode * node)
{
	for (int i = 0; i < TimeLines.size(); i++)
	{
		if (TimeLines[i]->Resource == node)
		{
			return TimeLines[i];
		}
	}
	ResourceTimeLine* newline = new ResourceTimeLine();
	newline->Resource = node;
	TimeLines.push_back(newline);
	return newline;
}

ResourceTimeLine::~ResourceTimeLine()
{
	MemoryUtils::DeleteVector(Frames);
}
