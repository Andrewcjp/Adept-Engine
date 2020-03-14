#pragma once
#include "..\RenderNode.h"

class InterGPUCopyNode : public RenderNode
{
public:
	enum CopyMode
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
	bool CopyTo = true;
	NameNode("InterGPUCopy");
protected:
	virtual void OnNodeSettingChange() override;


	virtual void OnSetupNode() override;
	RHICommandList* CopyList = nullptr;
};

