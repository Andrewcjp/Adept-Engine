#include "InterGPUCopyNode.h"
#include "Rendering/Core/FrameBuffer.h"
#include "Rendering/RenderNodes/NodeLink.h"
#include "Rendering/RenderNodes/StoreNodes/InterGPUStorageNode.h"
#include "../StorageNodeFormats.h"
#include "RHI/RHICommandList.h"
#include "RHI/SFRController.h"
#include "../../Core/Screen.h"
#include "RHI/RHITexture.h"
#include "RHI/DeviceContext.h"


InterGPUCopyNode::InterGPUCopyNode(DeviceContext* con)
{
	Context = con;
	NodeEngineType = ECommandListType::Copy;
	NodeQueueType = EDeviceContextQueue::InterCopy;
	OnNodeSettingChange();
}


InterGPUCopyNode::~InterGPUCopyNode()
{
}

void InterGPUCopyNode::ExecuteCopyTo(RHICommandList* List, InterGPUStorageNode* Node, RHIInterGPUStagingResource* InterRes)
{
	if (GetInput(0)->IsValid())
	{
		FrameBuffer* FB = GetFrameBufferFromInput(0);
		FB->GetRenderTexture()->CopyToStagingResource(InterRes, List, SFRController::GetScissor(1, Screen::GetScaledRes()));
		List->ResolveTimers();
	}
	if (GetInput(2)->IsValid())
	{
		RHIBuffer* Buffer = GetBufferFromInput(2);
		Buffer->CopyToStagingResource(InterRes, List);
		List->ResolveTimers();
	}

}
void InterGPUCopyNode::ExecuteCopyFrom(RHICommandList* List, InterGPUStorageNode* Node, RHIInterGPUStagingResource* InterRes)
{
	if (GetInput(0)->IsValid())
	{
		FrameBuffer* FB = GetFrameBufferFromInput(0);
		FB->GetRenderTexture()->CopyFromStagingResource(InterRes, List, SFRController::GetScissor(1, Screen::GetScaledRes()));
		CopyList->ResolveTimers();
	}
	if (GetInput(2)->IsValid())
	{
		RHIBuffer* Buffer = GetBufferFromInput(2);
		Buffer->CopyFromStagingResource(InterRes, List);
		List->ResolveTimers();
	}
}

void InterGPUCopyNode::ExecuteDirectCopy(RHICommandList* List, InterGPUStorageNode* Node, RHIInterGPUStagingResource* InterRes)
{

}

void InterGPUCopyNode::OnExecute()
{
	InterGPUStorageNode* Node = (InterGPUStorageNode*)GetInput(1)->GetStoreTarget();
	RHIInterGPUStagingResource* InterRes = Node->GetStore(0)->Resource;
	CopyList->ResetList();
	SetBeginStates(CopyList);
	if (Mode == CopyFromStage)
	{
		ExecuteCopyFrom(CopyList, Node, InterRes);
	}
	else if (Mode == CopyToStage)
	{
		ExecuteCopyTo(CopyList, Node, InterRes);
	}
	else if (Mode == CopyAcross)
	{
		ExecuteDirectCopy(CopyList, Node, InterRes);
	}
	SetEndStates(CopyList);
	CopyList->Execute(EDeviceContextQueue::InterCopy);
}

bool InterGPUCopyNode::IsNodeSupported(const RenderSettings& settings)
{
	return RHI::SupportsExplictMultiAdaptor();
}

void InterGPUCopyNode::OnNodeSettingChange()
{
	AddResourceInput(EStorageType::Framebuffer, EResourceState::Common, StorageFormats::DontCare, "Buffer to copy");
	AddInput(EStorageType::InterGPUStagingResource, StorageFormats::DontCare, "Staging resource");
	AddResourceInput(EStorageType::Buffer, EResourceState::Common, StorageFormats::DontCare, "Buffer to copy");
}

void InterGPUCopyNode::OnSetupNode()
{
	CopyList = RHI::CreateCommandList(ECommandListType::Copy, Context);
}
