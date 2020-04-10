#pragma once
#include "..\RenderNode.h"

class InterGPUStorageNode;

class InterGPUCopyNode : public RenderNode
{
public:
	enum ECopyMode
	{
		CopyToStage,
		CopyFromStage,
		CopyAcross,
		PredicatedCopyAcross,
		Limit
	};
	InterGPUCopyNode(DeviceContext* con);
	~InterGPUCopyNode();


	virtual void OnExecute() override;


	virtual bool IsNodeSupported(const RenderSettings& settings) override;
	InterGPUCopyNode::ECopyMode Mode = CopyFromStage;
	NameNode("InterGPUCopy");
	SFRNode* CopyNode;
protected:
	virtual void OnNodeSettingChange() override;
	void ExecuteCopyTo(RHICommandList* List, InterGPUStorageNode* Node, RHIInterGPUStagingResource* InterRes);
	void ExecuteCopyFrom(RHICommandList* List, InterGPUStorageNode* Node, RHIInterGPUStagingResource* InterRes);
	void ExecuteDirectCopy(RHICommandList* List, InterGPUStorageNode* Node, RHIInterGPUStagingResource* InterRes);
	virtual void OnSetupNode() override;
	RHICommandList* CopyList = nullptr;
};

