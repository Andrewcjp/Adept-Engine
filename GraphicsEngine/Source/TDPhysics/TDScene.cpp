#include "Stdafx.h"
#include "TDScene.h"
#include "TDQuadTree.h"
#include "TDRigidDynamic.h"
namespace TD
{
	TDScene::TDScene()
	{
		AcclerationTree = new TDQuadTree();
	}


	TDScene::~TDScene()
	{
		SafeDelete(AcclerationTree);
	}

	void TDScene::AddToScene(TDActor * Actor)
	{
		SceneActors.push_back(Actor);
		Actor->OwningScene = this;
		TDRigidDynamic* Dynamic = TDActor::ActorCast<TDRigidDynamic>(Actor);
		if (Dynamic != nullptr)
		{
			DynamicActors.push_back(Dynamic);
		}
	}
}