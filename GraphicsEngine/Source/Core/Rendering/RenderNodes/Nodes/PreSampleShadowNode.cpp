#include "Stdafx.h"
#include "PreSampleShadowNode.h"
#include "../StorageNodeFormats.h"


PreSampleShadowNode::PreSampleShadowNode()
{
	OnNodeSettingChange();
}


PreSampleShadowNode::~PreSampleShadowNode()
{}

void PreSampleShadowNode::OnExecute()
{

}

void PreSampleShadowNode::OnNodeSettingChange()
{
	AddInput(EStorageType::Framebuffer, StorageFormats::DefaultFormat);
	AddInput(EStorageType::ShadowData, StorageFormats::ShadowData);

	AddOutput(EStorageType::Framebuffer, StorageFormats::PreSampleShadowData);
}

void PreSampleShadowNode::OnSetupNode()
{
	Commandlist = RHI::CreateCommandList(ECommandListType::Graphics, Context);
}
