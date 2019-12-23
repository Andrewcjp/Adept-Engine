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
};
class RenderGraphProcessor
{
public:
	RenderGraphProcessor();
	~RenderGraphProcessor();
	 void Process(RenderGraph* graph);
private:
	void BuildAliasing(RenderGraph * graph);
	void BuildTimeLine(RenderGraph * graph);
	void BuildScheduling(RenderGraph* graph);
	void BuildTransitions(RenderGraph * graph);
	std::vector<ResourceTimeLine*> TimeLines;
	ResourceTimeLine* GetOrCreateTimeLine(StorageNode* node);
};

