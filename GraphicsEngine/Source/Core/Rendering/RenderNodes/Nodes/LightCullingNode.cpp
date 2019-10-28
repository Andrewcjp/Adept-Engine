
#include "LightCullingNode.h"
#include "..\..\Core\LightCulling\LightCullingEngine.h"


LightCullingNode::LightCullingNode()
{
	NodeEngineType = ENodeQueueType::Compute;
	SetNodeActive(false);
}


LightCullingNode::~LightCullingNode()
{}

void LightCullingNode::OnExecute()
{
	List->ResetList();
	SceneRenderer::Get()->GetLightCullingEngine()->LaunchCullingForScene(List, EEye::Left);
	List->Execute();

	RHI::GetDefaultDevice()->InsertGPUWait(DeviceContextQueue::Graphics, DeviceContextQueue::Compute);
}

std::string LightCullingNode::GetName() const
{
	return "Light Culling";
}

void LightCullingNode::OnNodeSettingChange()
{

}

void LightCullingNode::OnSetupNode()
{
	List = RHI::CreateCommandList(ECommandListType::Compute, Context);
}
