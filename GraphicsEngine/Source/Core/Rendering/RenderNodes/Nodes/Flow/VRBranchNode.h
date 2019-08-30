#pragma once
#include "Rendering/RenderNodes/RenderNode.h"
#include "Rendering/VR/HMD.h"
class VRBranchNode : public RenderNode
{
public:
	VRBranchNode();
	~VRBranchNode();

	virtual void OnExecute() override;
	VRBranchNode* VrLoopBegin = nullptr;
	EEye::Type CurrentEyeMode = EEye::Left;

	NameNode("VR Branch");
	EEye::Type GetCurrentEye();
protected:
	virtual void OnSetupNode() override;

};

