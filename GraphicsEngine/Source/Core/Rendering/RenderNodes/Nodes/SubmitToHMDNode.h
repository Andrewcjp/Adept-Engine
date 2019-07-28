#pragma once
#include "Rendering/RenderNodes/RenderNode.h"

class SubmitToHMDNode : public RenderNode
{
public:
	SubmitToHMDNode();
	~SubmitToHMDNode();

	virtual void OnExecute() override;
	virtual bool IsNodeSupported(const RenderSettings& settings) override;
protected:
	virtual void OnNodeSettingChange() override;
	virtual void OnValidateNode(RenderGraph::ValidateArgs & args) override;
};

