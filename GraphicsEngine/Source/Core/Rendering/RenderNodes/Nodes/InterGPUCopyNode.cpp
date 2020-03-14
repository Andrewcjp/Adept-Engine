#include "InterGPUCopyNode.h"
#include "Rendering/Core/FrameBuffer.h"
#include "Rendering/RenderNodes/NodeLink.h"
#include "Rendering/RenderNodes/StoreNodes/InterGPUStorageNode.h"
#include "../StorageNodeFormats.h"
#include "RHI/RHICommandList.h"


InterGPUCopyNode::InterGPUCopyNode(DeviceContext* con)
{
	Context = con;
	NodeEngineType = ECommandListType::Copy;
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
		FB->CopyToStagingResource(InterRes, CopyList);
	}
	else
	{
		FB->CopyFromStagingResource(InterRes, CopyList);
	}
	SetEndStates(CopyList);
	CopyList->Execute();
}

bool InterGPUCopyNode::IsNodeSupported(const RenderSettings& settings)
{
	return RHI::SupportsExplictMultiAdaptor();
}

void InterGPUCopyNode::OnNodeSettingChange()
{
	AddResourceInput(EStorageType::Framebuffer,EResourceState::Common, StorageFormats::DontCare, "Buffer to copy");
	AddInput(EStorageType::InterGPUStagingResource, StorageFormats::DontCare, "Staging resource");
}

void InterGPUCopyNode::OnSetupNode()
{
	CopyList = RHI::CreateCommandList(ECommandListType::Copy, Context);
}
