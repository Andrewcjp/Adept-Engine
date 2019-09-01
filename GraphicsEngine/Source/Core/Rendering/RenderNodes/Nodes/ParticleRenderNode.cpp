#include "ParticleRenderNode.h"
#include "Rendering/Core/ParticleSystemManager.h"

#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "../NodeLink.h"

ParticleRenderNode::ParticleRenderNode()
{
	ViewMode = EViewMode::PerView;
	OnNodeSettingChange();
}

ParticleRenderNode::~ParticleRenderNode()
{}

void ParticleRenderNode::OnExecute()
{
	FrameBuffer* Buffer = GetFrameBufferFromInput(0);
	FrameBuffer* DepthSource = nullptr;
	if (GetInput(1)->IsValid())
	{
		DepthSource = GetFrameBufferFromInput(1);
	}
	ParticleSystemManager::Get()->Render(Buffer, DepthSource, GetEye());
	PassNodeThough(0);
}

void ParticleRenderNode::OnNodeSettingChange()
{
	AddInput(EStorageType::Framebuffer, StorageFormats::LitScene, "Screen Data");
	AddInput(EStorageType::Framebuffer, StorageFormats::GBufferData, "Depth Data");
	AddOutput(EStorageType::Framebuffer, StorageFormats::LitScene);
}
