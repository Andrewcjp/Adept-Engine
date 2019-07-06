#pragma once
#include "../../RenderNode.h"
#include "../../../VR/HMD.h"
class VRBranchNode : public RenderNode
{
public:
	VRBranchNode();
	~VRBranchNode();

	virtual void OnExecute() override;
	VRBranchNode* VrLoopBegin = nullptr;
	EEye::Type CurrentEyeMode = EEye::Left;

	virtual std::string GetName() const override;

protected:
	virtual void OnSetupNode() override;

};

