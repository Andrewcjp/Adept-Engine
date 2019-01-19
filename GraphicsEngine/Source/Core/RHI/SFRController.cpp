#include "Source/Core/Stdafx.h"
#include "SFRController.h"
#include "RHI.h"


SFRController::SFRController()
{
	Init();
}

SFRController::~SFRController()
{}

void SFRController::Tick()
{
	//todo: dynamically readjust SFR splits
}

SFRNode * SFRController::GetNode(int DeviceIndex)
{
	return Nodes[DeviceIndex];
}

void SFRController::Init()
{
	float SplitPc = 1.0f / RHI::GetDeviceCount();
	for (int i = 0; i < RHI::GetDeviceCount(); i++)
	{
		SFRNode* Node = new SFRNode();
		Node->SFR_PercentSize = SplitPc;
		Node->SFR_Offset = i * SplitPc;
		Nodes.push_back(Node);
	}
}
