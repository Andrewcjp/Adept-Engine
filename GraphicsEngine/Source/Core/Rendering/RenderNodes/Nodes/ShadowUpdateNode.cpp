#include "ShadowUpdateNode.h"
#include "Core/Assets/Scene.h"
#include "Rendering/Core/SceneRenderer.h"
#include "Rendering/Core/ShadowRenderer.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "Rendering/RenderNodes/StoreNodes/ShadowAtlasStorageNode.h"

ShadowUpdateNode::ShadowUpdateNode()
{
	OnNodeSettingChange();
}


ShadowUpdateNode::~ShadowUpdateNode()
{}

void ShadowUpdateNode::OnExecute()
{
	ShadowAtlasStorageNode* Node = GetShadowDataFromInput(0);

	//mShadowRenderer->RenderShadowMaps();
	//ShadowRenderer::AssignAtlasData(Node->GetAtlas());

	ShadowList->ResetList();
	ShadowRenderer::RenderPointShadows(ShadowList);
	ShadowList->Execute();
}

void ShadowUpdateNode::OnNodeSettingChange()
{
	AddInput(EStorageType::ShadowData, StorageFormats::ShadowData);
}

void ShadowUpdateNode::OnSetupNode()
{
	//mShadowRenderer = new ShadowRenderer(SceneRenderer::Get(), SceneRenderer::Get()->GetCullingManager());
	////if (MainScene != nullptr)
	//{
	//	mShadowRenderer->InitShadows(SceneRenderer::Get()->GetScene()->GetLights());
	//}
	ShadowList = RHI::CreateCommandList(ECommandListType::Graphics, Context);
}
