#pragma once
#include "../RenderNode.h"

class RHICommandList;
//this node renders the scene with debug visualizations   
class VisModeNode : public RenderNode
{
public:
	VisModeNode();
	~VisModeNode();

	virtual void OnExecute() override;


	virtual std::string GetName() const override;

protected:
	virtual void OnNodeSettingChange() override;
	virtual void OnSetupNode() override;

	void RenderGBufferModes(ERenderDebugOutput::Type mode);
	void RenderForwardMode(ERenderDebugOutput::Type mode);
	void RenderScreenSpaceModes(ERenderDebugOutput::Type mode);
private:
	RHICommandList* DebugList = nullptr;
};

