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
	ensure(VrLoopBegin);
	if (VrLoopBegin->CurrentEyeMode == EEye::Left)
	{
		VrLoopBegin->CurrentEyeMode = EEye::Right;
		VrLoopBegin->ExecuteNode();
	}
	else
	{
		if (Next != nullptr)
		{
			Next->ExecuteNode();
		}
	}

}

void VRBranchNode::OnSetupNode()
{

}
