
#include "SFRController.h"
#include "RHI.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Utils/MathUtils.h"
#include "Rendering/Core/FrameBuffer.h"


SFRController::SFRController()
{
	Init();
	AllowDynmaicChanging = false;
	DynamicAdjustSpeed = 0.5f;
	CurrentCoolDown = DynamicAdjustSpeed;
}

SFRController::~SFRController()
{}

void SFRController::Tick()
{
	if (!AllowDynmaicChanging)
	{
		return;
	}
	CurrentCoolDown -= Engine::GetDeltaTime();
	if (CurrentCoolDown > 0.0f)
	{
		return;
	}
	CurrentCoolDown = DynamicAdjustSpeed;
	float GPUTime0 = Nodes[0]->SFRDriveTimer->AVG->GetCurrentAverage();
	float GPUTime1 = Nodes[1]->SFRDriveTimer->AVG->GetCurrentAverage();
	const float CloseEnought = 0.15f;
	if (MathUtils::AlmostEqual(GPUTime0, GPUTime1, CloseEnought))
	{
		return;
	}
	float Delta = (GPUTime1 - GPUTime0);
	Delta = GPUTime0 / GPUTime1;
	Delta = 1 - Delta;
	Delta = glm::clamp(Delta, -PCMaxdelta, PCMaxdelta);
	CurrnetPC += Delta;
	DualUpdatePC(CurrnetPC);
	Resize();
}
void SFRController::Resize()
{
	for (int i = 0; i < RHI::GetDeviceCount(); i++)
	{
		for (int x = 0; x < Nodes[i]->NodesBuffers.size(); x++)
		{
			Nodes[i]->NodesBuffers[x]->SFRResize();
		}
	}
}
void SFRController::DualUpdatePC(float splittest)
{
	splittest = glm::clamp(splittest, 0.01f, 0.99f);
	if (splittest == LastPc)
	{
		return;
	}
	Nodes[0]->SFR_PercentSize = splittest;
	Nodes[0]->SFR_Offset = 0.0f;

	Nodes[1]->SFR_PercentSize = 1.0f - splittest;
	Nodes[1]->SFR_Offset = splittest;
	CurrnetPC = splittest;
	LastPc = CurrnetPC;


	//	if (RHI::GetMGPUSettings()->MainPassSFR || RHI::GetMGPUSettings()->SFRSplitShadows)
	{
		Log::LogMessage("SFR updated to " + std::to_string(CurrnetPC));
	}
}

SFRNode* SFRController::GetNode(int DeviceIndex)
{
	return Nodes[DeviceIndex];
}

void SFRController::Init()
{
	if (RHI::GetDeviceCount() == 1)
	{
		return;
	}
	//#SFR: rewrite
	float SplitPc = 1.0f / RHI::GetDeviceCount();
	for (int i = 0; i < RHI::GetDeviceCount(); i++)
	{
		SFRNode* Node = new SFRNode();
		Node->SFR_PercentSize = SplitPc;
		Node->SFR_Offset = i * SplitPc;
		Node->SFRDriveTimer = PerfManager::Get()->GetTimerData(PerfManager::Get()->GetTimerIDByName("Main Pass" + std::to_string(i)));
		Nodes.push_back(Node);
	}
	const float splittest = 0.9f;
	DualUpdatePC(splittest);
}

RHIScissorRect SFRController::GetScissor(int index, glm::ivec2 Res)
{
	SFRNode* n = RHI::GetSplitController()->GetNode(index);
	const int Offset = n->SFR_Offset * Res.x;
	const int right = Offset + (Res.x * n->SFR_PercentSize);
	RHIScissorRect rect = RHIScissorRect(Offset, 0, right, Res.y);
	if (index == 0)
	{
		rect.Right -= 10;
	}
	return rect;
}
