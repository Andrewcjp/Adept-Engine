#include "MultiGPULoopNode.h"
#include "Rendering/RenderNodes/RenderNode.h"


MultiGPULoopNode::MultiGPULoopNode()
{}


MultiGPULoopNode::~MultiGPULoopNode()
{}

void MultiGPULoopNode::SetLoopBody(LoopBodyFunction func)
{
	LoopBodyFunctionPtr = func;
}

void MultiGPULoopNode::OnExecute()
{

}


void MultiGPULoopNode::OnGraphCreate()
{
	RenderNode* Before = LastNode;
	for (int i = 0; i < RHI::GetDeviceCount(); i++)
	{
		RenderNode* itor = Before;
		Before = LoopBodyFunctionPtr(Before, RHI::GetDeviceContext(i));
		//todo: handle device zero copy
		if (i > 0)
		{
			itor = itor->GetNextNode();
			while (itor != nullptr && itor != Next)
			{
				itor->SetDevice(RHI::GetDeviceContext(i));
				itor = itor->GetNextNode();
			}
		}
	}
	Before->LinkToNode(Next);
}

void MultiGPULoopNode::OnSetupNode()
{

}
