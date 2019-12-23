#include "ParticleSimulateNode.h"
#include "Rendering/Core/ParticleSystemManager.h"


ParticleSimulateNode::ParticleSimulateNode()
{
	NodeEngineType = ECommandListType::Compute;
	ViewMode = EViewMode::DontCare;
	//SetNodeActive(false);
}


ParticleSimulateNode::~ParticleSimulateNode()
{}

void ParticleSimulateNode::OnExecute()
{
	HasRunBegin = true;
	HasRunEnd = true;
	ParticleSystemManager::Get()->Simulate();
}
