#include "VRLoopNode.h"
#include "../../RenderNode.h"


VRLoopNode::VRLoopNode()
{}


VRLoopNode::~VRLoopNode()
{}

void VRLoopNode::SetLoopBody(LoopBodyFunction func)
{
	LoopBodyFunctionPtr = func;
}

void VRLoopNode::OnExecute()
{
	
}


void VRLoopNode::OnGraphCreate()
{
	RenderNode* Before = LastNode;
	for (int i = 0; i < 2; i++)
	{
		RenderNode* itor = Before;
		Before = LoopBodyFunctionPtr(Before);
		if (i == 1)
		{			
			while (itor != nullptr)
			{
				itor->SetTargetEye(EEye::Right);
				itor = itor->GetNextNode();
			}
		}
	}
	Before->LinkToNode(Next);
}

void VRLoopNode::OnSetupNode()
{

}
