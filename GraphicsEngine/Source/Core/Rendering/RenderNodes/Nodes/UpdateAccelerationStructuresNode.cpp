#include "UpdateAccelerationStructuresNode.h"
#include "..\..\RayTracing\RayTracingEngine.h"


UpdateAccelerationStructuresNode::UpdateAccelerationStructuresNode()
{
	NodeEngineType = ECommandListType::Compute;
	OnNodeSettingChange();
}

UpdateAccelerationStructuresNode::~UpdateAccelerationStructuresNode()
{}

void UpdateAccelerationStructuresNode::OnExecute()
{
	RHICommandList* list = Context->GetListPool()->GetCMDList(ECommandListType::Compute);
	SetBeginStates(list);
	RayTracingEngine::Get()->BuildStructures(list);
	SetEndStates(list);
	Context->GetListPool()->Flush();
	RHI::GetDefaultDevice()->InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::Compute);
}

void UpdateAccelerationStructuresNode::OnNodeSettingChange()
{
	//#DXR: todo
}

bool UpdateAccelerationStructuresNode::IsNodeSupported(const RenderSettings& settings)
{
	if (Context->GetCaps().RTSupport == ERayTracingSupportType::None)
	{
		return false;
	}
	if (!settings.RaytracingEnabled())
	{
		return false;
	}
	return true;
}
