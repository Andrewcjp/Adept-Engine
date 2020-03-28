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
	NodeQueueType = DeviceContextQueue::InterCopy;
	OnNodeSettingChange();
}


InterGPUCopyNode::~InterGPUCopyNode()
{}

void InterGPUCopyNode::OnExecute()
{
	FrameBuffer* FB = GetFrameBufferFromInput(0);
	InterGPUStorageNode* Node = (InterGPUStorageNode*)GetInput(1)->GetStoreTarget();
	RHIInterGPUStagingResource* InterRes = Node->GetStore(0)->Resource;
	CopyList->ResetList();
	SetBeginStates(CopyList);
	//todo: target sub resources in FBs
	if (CopyTo)
	{
		
		//FB->CopyToStagingResource(InterRes, CopyList);
		FB->GetRenderTexture()->CopyToStagingResource(InterRes, CopyList, SFRController::GetScissor(1, Screen::GetScaledRes()));
		CopyList->ResolveTimers();
	}
	else
	{
		//if (CopyNode != nullptr)
		{
			FB->GetRenderTexture()->CopyFromStagingResource(InterRes, CopyList, SFRController::GetScissor(1, Screen::GetScaledRes()));
		}
		/*else
		{
			FB->CopyFromStagingResource(InterRes, CopyList);
		}*/
		CopyList->ResolveTimers();
	}
	SetEndStates(CopyList);
	CopyList->Execute(DeviceContextQueue::InterCopy);
}

bool InterGPUCopyNode::IsNodeSupported(const RenderSettings& settings)
{
	return RHI::SupportsExplictMultiAdaptor();
}

void InterGPUCopyNode::OnNodeSettingChange()
{
	AddResourceInput(EStorageType::Framebuffer, EResourceState::Common, StorageFormats::DontCare, "Buffer to copy");
	AddInput(EStorageType::InterGPUStagingResource, StorageFormats::DontCare, "Staging resource");
}

void InterGPUCopyNode::OnSetupNode()
{
	CopyList = RHI::CreateCommandList(ECommandListType::Copy, Context);
}
