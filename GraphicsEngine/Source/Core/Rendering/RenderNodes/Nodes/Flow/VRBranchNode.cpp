#include "VRBranchNode.h"


VRBranchNode::VRBranchNode()
{
	NodeControlsFlow = true;
	IsVrBranchNode = true;
}


VRBranchNode::~VRBranchNode()
{}

void VRBranchNode::OnExecute()
{
	//ensure(VrLoopBegin);
	if (VrLoopBegin != nullptr && VrLoopBegin->CurrentEyeMode == EEye::Left)
	{
		VrLoopBegin->CurrentEyeMode = EEye::Right;
		VrLoopBegin->ExecuteNode();
	}
	else
	{
		if (VrLoopBegin != nullptr)
		{
			VrLoopBegin->CurrentEyeMode = EEye::Left;
		}
		if (Next != nullptr)
		{
			Next->ExecuteNode();
		}
	}

}

EEye::Type VRBranchNode::GetCurrentEye()
{
	if (VrLoopBegin != nullptr)
	{
		return VrLoopBegin->CurrentEyeMode;
	}
	return CurrentEyeMode;
}

void VRBranchNode::OnSetupNode()
{

}
