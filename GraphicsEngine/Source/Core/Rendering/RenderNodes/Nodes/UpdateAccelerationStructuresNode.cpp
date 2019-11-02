#include "UpdateAccelerationStructuresNode.h"
#include "..\..\RayTracing\RayTracingEngine.h"


UpdateAccelerationStructuresNode::UpdateAccelerationStructuresNode()
{
	OnNodeSettingChange();
}

UpdateAccelerationStructuresNode::~UpdateAccelerationStructuresNode()
{}

void UpdateAccelerationStructuresNode::OnExecute()
{
	RayTracingEngine::Get()->BuildStructures();
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
