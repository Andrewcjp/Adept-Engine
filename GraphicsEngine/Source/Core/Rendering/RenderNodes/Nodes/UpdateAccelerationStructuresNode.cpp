#include "UpdateAccelerationStructuresNode.h"
#include "..\..\RayTracing\RayTracingEngine.h"
#include "RHI\CommandListPool.h"
#include "RHI\DeviceContext.h"
#include "Rendering\Core\SceneRenderer.h"
#include "Rendering\RayTracing\RayTracingScene.h"


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
#if 0
	RayTracingEngine::Get()->BuildStructures(list);
#else
	SceneRenderer::Get()->GetRTScene()->ProcessUpdatesToAcclerationStuctures(list);
#endif
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
