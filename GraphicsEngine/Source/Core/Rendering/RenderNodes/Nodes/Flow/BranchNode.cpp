#include "BranchNode.h"


BranchNode::BranchNode()
{
	NodeControlsFlow = true;
}


BranchNode::~BranchNode()
{}

void BranchNode::OnExecute()
{
	if (Conditonal)
	{
		if (Next != nullptr)
		{
			Next->ExecuteNode();
		}
	}
	else
	{
		if (BranchB != nullptr)
		{
			BranchB->ExecuteNode();
		}
	}
}
