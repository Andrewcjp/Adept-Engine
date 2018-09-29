#include "Stdafx.h"
#include "TDScene.h"
#include "TDQuadTree.h"
namespace TD
{
	TDScene::TDScene()
	{
		AcclerationTree = new TDQuadTree();
	}


	TDScene::~TDScene()
	{}

	void TDScene::AddToScene(TDActor * Actor)
	{
		SceneActors.push_back(Actor);
		Actor->OwningScene = this;
	}
}