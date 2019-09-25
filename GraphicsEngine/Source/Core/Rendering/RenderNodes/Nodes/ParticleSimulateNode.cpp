#include "ParticleSimulateNode.h"
#include "Rendering/Core/ParticleSystemManager.h"


ParticleSimulateNode::ParticleSimulateNode()
{
	NodeEngineType = ENodeQueueType::Compute;
	ViewMode = EViewMode::DontCare;
	//SetNodeActive(false);
}


ParticleSimulateNode::~ParticleSimulateNode()
{}

void ParticleSimulateNode::OnExecute()
{
	ParticleSystemManager::Get()->Simulate();
}
