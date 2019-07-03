#include "ParticleSimulateNode.h"
#include "Rendering/Core/ParticleSystemManager.h"


ParticleSimulateNode::ParticleSimulateNode()
{
	NodeEngineType = ENodeQueueType::Compute;
	ViewMode = EViewMode::DontCare;
}


ParticleSimulateNode::~ParticleSimulateNode()
{}

void ParticleSimulateNode::OnExecute()
{
	ParticleSystemManager::Get()->Simulate();
}
