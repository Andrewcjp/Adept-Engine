
#include "ForwardRenderNode.h"
#include "../NodeLink.h"
#include "../StorageNodeFormats.h"


ForwardRenderNode::ForwardRenderNode()
{
	AddInput(EStorageType::Framebuffer, StorageFormats::DefaultFormat, "Output buffer");
	AddOutput(EStorageType::Framebuffer, StorageFormats::LitScene, "Lit output");
}


ForwardRenderNode::~ForwardRenderNode()
{}

void ForwardRenderNode::OnExecute()
{

}

void ForwardRenderNode::SetupNode()
{

}

std::string ForwardRenderNode::GetName() const
{
	return "Forward Pass";
}
