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
