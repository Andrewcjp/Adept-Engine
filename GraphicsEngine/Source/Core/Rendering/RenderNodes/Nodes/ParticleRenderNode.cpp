#include "ParticleRenderNode.h"
#include "Rendering/Core/ParticleSystemManager.h"

#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "../NodeLink.h"

ParticleRenderNode::ParticleRenderNode()
{
	ViewMode = EViewMode::PerView;
	OnNodeSettingChange();
	if (!RHI::GetRenderSettings()->EnableGPUParticles)
	{
		SetNodeActive(false);
	}
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
	RenderList->ResetList();
	RenderList->StartTimer(EGPUTIMERS::ParticleDraw);
	ParticleSystemManager::Get()->Render(Buffer, DepthSource, GetEye());
	RenderList->EndTimer(EGPUTIMERS::ParticleDraw);
	SetEndStates(RenderList);
	RenderList->Execute();
	PassNodeThough(0);
}

void ParticleRenderNode::OnNodeSettingChange()
{
	AddInput(EStorageType::Framebuffer, StorageFormats::LitScene, "Screen Data");
	AddInput(EStorageType::Framebuffer, StorageFormats::GBufferData, "Depth Data");
	AddOutput(EStorageType::Framebuffer, StorageFormats::LitScene);
}

void ParticleRenderNode::OnSetupNode()
{
	RenderList = RHI::CreateCommandList(ECommandListType::Graphics, Context);
	ParticleSystemManager::Get()->RenderList = RenderList;
}
