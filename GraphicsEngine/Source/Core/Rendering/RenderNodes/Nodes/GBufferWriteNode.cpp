#include "GBufferWriteNode.h"
#include "Rendering/RenderNodes/NodeLink.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"


GBufferWriteNode::GBufferWriteNode()
{
	ViewMode = EViewMode::PerView;
	AddInput(EStorageType::Framebuffer, StorageFormats::DefaultFormat);
	AddOutput(EStorageType::Framebuffer, StorageFormats::GBufferData);
}

GBufferWriteNode::~GBufferWriteNode()
{}

void GBufferWriteNode::OnExecute()
{
	ensure(GetInput(0)->GetStoreTarget());

	GetInput(0)->GetStoreTarget()->DataFormat = StorageFormats::GBufferData;
	//pass the input to the output now changed
	GetOutput(0)->SetStore(GetInput(0)->GetStoreTarget());
}

void GBufferWriteNode::OnSetupNode()
{

}
