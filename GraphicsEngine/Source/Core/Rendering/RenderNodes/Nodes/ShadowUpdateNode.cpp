
#include "ShadowUpdateNode.h"
#include "../StorageNodeFormats.h"
#include "../../Core/SceneRenderer.h"
#include "../../Core/ShadowRenderer.h"
#include "Core/Assets/Scene.h"
#include "../StoreNodes/ShadowAtlasStorageNode.h"


ShadowUpdateNode::ShadowUpdateNode()
{
	OnNodeSettingChange();
}


ShadowUpdateNode::~ShadowUpdateNode()
{}

void ShadowUpdateNode::OnExecute()
{
	ShadowAtlasStorageNode* Node = GetShadowDataFromInput(0);
	if (Once)
	{
		mShadowRenderer = new ShadowRenderer(SceneRenderer::Get(), SceneRenderer::Get()->GetCullingManager());
		mShadowRenderer->InitShadows(SceneRenderer::Get()->GetScene()->GetLights());

		Once = false;
	}
	Node->Shadower = mShadowRenderer;
	mShadowRenderer->RenderShadowMaps();
}

std::string ShadowUpdateNode::GetName() const
{
	return "Shadow Update";
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
}
