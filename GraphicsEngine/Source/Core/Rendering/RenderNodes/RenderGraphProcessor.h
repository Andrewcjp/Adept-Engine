#pragma once
#include "RHI/RHITypes.h"

class RenderNode;
class StorageNode;
class NodeLink;
struct ResourceTimelineFrame
{
	EResourceState::Type State = EResourceState::Limit;
	RenderNode* Node = nullptr;
	NodeLink* TargetLink = nullptr;
};
struct ResourceTimeLine
{
	std::vector<ResourceTimelineFrame*> Frames;
	StorageNode* Resource = nullptr;
	~ResourceTimeLine();
};
class RenderGraphProcessor
{
public:
	RenderGraphProcessor();
	~RenderGraphProcessor();
	 void Process(RenderGraph* graph);
	 void Reset();
private:
	void BuildAliasing(RenderGraph * graph);
	void BuildTimeLine(RenderGraph * graph);
	void BuildScheduling(RenderGraph* graph);
	void BuildScheduling_old(RenderGraph * graph);
	void BuildTransitions(RenderGraph * graph);
	std::vector<ResourceTimeLine*> TimeLines;
	void BuildTransitionsSplit(RenderGraph * graph);
	ResourceTimeLine* GetOrCreateTimeLine(StorageNode* node);
	int NodeCount = 0;
};

