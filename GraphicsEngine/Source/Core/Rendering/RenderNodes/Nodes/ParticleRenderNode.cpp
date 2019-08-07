#include "ParticleRenderNode.h"
#include "Rendering/Core/ParticleSystemManager.h"

#include "Rendering/RenderNodes/StorageNodeFormats.h"

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
	//DeviceDependentObjects D;
	//D.MainFrameBuffer = Buffer;
	ParticleSystemManager::Get()->Render(Buffer);
	PassNodeThough(0);
}

void ParticleRenderNode::OnNodeSettingChange()
{
	AddInput(EStorageType::Framebuffer, StorageFormats::LitScene);
	AddOutput(EStorageType::Framebuffer, StorageFormats::LitScene);
}
