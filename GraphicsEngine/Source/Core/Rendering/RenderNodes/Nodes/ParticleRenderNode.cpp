#include "ParticleRenderNode.h"
#include "Rendering/Core/ParticleSystemManager.h"

#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "../NodeLink.h"
#include "RHI/RHICommandList.h"

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
	SetBeginStates(RenderList);
	ParticleSystemManager::Get()->RenderList = RenderList;
	ParticleSystemManager::Get()->Render(Buffer, DepthSource, GetEye());
	RenderList->EndTimer(EGPUTIMERS::ParticleDraw);
	SetEndStates(RenderList);
	RenderList->Execute();
	PassNodeThough(0);
}

void ParticleRenderNode::OnNodeSettingChange()
{
	AddResourceInput(EStorageType::Framebuffer, EResourceState::RenderTarget, StorageFormats::LitScene, "Screen Data");
	AddResourceInput(EStorageType::Framebuffer, EResourceState::PixelShader, StorageFormats::GBufferData, "Depth Data")->SetOptional();
	AddOutput(EStorageType::Framebuffer, StorageFormats::LitScene);
	LinkThough(0);
}

void ParticleRenderNode::OnSetupNode()
{
	RenderList = RHI::CreateCommandList(ECommandListType::Graphics, Context);
	ParticleSystemManager::Get()->RenderList = RenderList;
}
