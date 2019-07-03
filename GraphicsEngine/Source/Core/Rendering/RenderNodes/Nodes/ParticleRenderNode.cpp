#include "ParticleRenderNode.h"
#include "../StorageNodeFormats.h"
#include "../../Renderers/RenderEngine.h"
#include "../../Core/ParticleSystemManager.h"

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
	DeviceDependentObjects D;
	D.MainFrameBuffer = Buffer;
	ParticleSystemManager::Get()->Render(&D);

}

void ParticleRenderNode::OnNodeSettingChange()
{
	AddInput(EStorageType::Framebuffer, StorageFormats::LitScene);
	AddOutput(EStorageType::Framebuffer, StorageFormats::LitScene);
}
