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

void VRBranchNode::OnSetupNode()
{

}
