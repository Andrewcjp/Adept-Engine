#pragma once
#include "../RenderNode.h"

class RHICommandList;
class Shader_Pair;
//this node renders the scene with debug visualizations   
class VisModeNode : public RenderNode
{
public:
	VisModeNode();
	~VisModeNode();

	virtual void OnExecute() override;


	NameNode("View Modes");


	void RefreshNode() override;

protected:
	virtual void OnNodeSettingChange() override;
	virtual void OnSetupNode() override;

	void VisTexturesimple(ERenderDebugOutput::Type mode);
	void RenderGBufferModes(ERenderDebugOutput::Type mode);
	void RenderForwardMode(ERenderDebugOutput::Type mode);
	void RenderScreenSpaceModes(ERenderDebugOutput::Type mode);
private:
	RHICommandList* DebugList = nullptr;
	Shader_Pair* Pair = nullptr;
};

