#include "DebugUINode.h"
#include "../StorageNodeFormats.h"


DebugUINode::DebugUINode()
{
	AddInput(EStorageType::Framebuffer, StorageFormats::LitScene, "Data");
	AddOutput(EStorageType::Framebuffer, StorageFormats::LitScene, "Final frame");
}

DebugUINode::~DebugUINode()
{}

void DebugUINode::OnExecute()
{

}

std::string DebugUINode::GetName() const
{
	return "DebugUI";
}
