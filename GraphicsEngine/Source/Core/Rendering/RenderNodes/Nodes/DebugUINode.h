#pragma once
#include "../RenderNode.h"
//handles Debug test Perf data and Lines.
class DebugUINode : public RenderNode
{
public:
	DebugUINode();
	~DebugUINode();

	virtual void OnExecute() override;
	virtual std::string GetName() const override;
	bool ClearBuffer = false;
private:
	RHICommandList* CommandList = nullptr;
protected:
	virtual void OnNodeSettingChange() override;


	virtual void OnSetupNode() override;

};

