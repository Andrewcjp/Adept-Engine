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

std::string UpdateAccelerationStructuresNode::GetName() const
{
	return "Update Acceleration Structures";
}

void UpdateAccelerationStructuresNode::OnNodeSettingChange()
{
	//#DXR: todo
}
