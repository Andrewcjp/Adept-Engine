#include "InterGPUCopyNode.h"
#include "Rendering/Core/FrameBuffer.h"
#include "Rendering/RenderNodes/NodeLink.h"
#include "Rendering/RenderNodes/StoreNodes/InterGPUStorageNode.h"
#include "../StorageNodeFormats.h"


InterGPUCopyNode::InterGPUCopyNode(DeviceContext* con)
{
	Context = con;
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
	if (CopyTo)
	{
		FB->CopyToStagingResource(InterRes, CopyList);
	}
	else
	{
		FB->CopyFromStagingResource(InterRes, CopyList);
	}
	CopyList->Execute();
}

bool InterGPUCopyNode::IsNodeSupported(const RenderSettings& settings)
{
	return RHI::SupportsExplictMultiAdaptor();
}

void InterGPUCopyNode::OnNodeSettingChange()
{
	AddInput(EStorageType::Framebuffer, StorageFormats::DontCare, "Buffer to copy");
	AddInput(EStorageType::InterGPUStagingResource, StorageFormats::DontCare, "Staging resource");
}

void InterGPUCopyNode::OnSetupNode()
{
	CopyList = RHI::CreateCommandList(ECommandListType::Copy, Context);
}
