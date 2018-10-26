#include "Stdafx.h"
#include "TDScene.h"
#include "TDOctTree.h"
#include "TDRigidDynamic.h"
namespace TD
{
	TDScene::TDScene()
	{
		AcclerationTree = new TDOctTree();
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
	template<class T>
	void RemoveFromVector(std::vector<T*>& vector, T* target) {
		for (int i = 0; i < vector.size(); i++) {
			if (vector[i] == target) {
				vector.erase(vector.begin() + i);
				return;
			}
		}
	}
	void TDScene::RemoveActor(TDActor * Actor)
	{
		RemoveFromVector(SceneActors, Actor);
		TDRigidDynamic* Dynamic = TDActor::ActorCast<TDRigidDynamic>(Actor);
		if (Dynamic != nullptr)
		{
			RemoveFromVector(DynamicActors, Dynamic);
		}		
	}

}