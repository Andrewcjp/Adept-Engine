#include "Stdafx.h"
#include "TDScene.h"
#include "TDOctTree.h"
#include "TDRigidDynamic.h"
#include "TDTypes.h"
#include "TDCollisionHandlers.h"
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

	bool TDScene::RayCastScene(glm::vec3 Origin, glm::vec3 Dir, float Distance, RaycastData * HitData)
	{
		RayCastSceneInternal(Origin, Dir, Distance, HitData);
		return false;
	}
	bool TDScene::RayCastSceneInternal(glm::vec3 Origin, glm::vec3 Dir, float Distance, RaycastData * HitData)
	{
		//todo: make go fast!
		//todo: MultiCast 
		bool Hit = false;
		for (int i = 0; i < SceneActors.size(); i++)
		{
			TDActor* actor = SceneActors[i];
			for (int j = 0; j < actor->GetAttachedShapes().size(); j++)
			{
				TDShape* currentshape = actor->GetAttachedShapes()[j];
				DebugEnsure(currentshape);
				IntersectionMethod con = IntersectionMethodTable[currentshape->GetShapeType()];
				DebugEnsure(con);
				Hit = con(currentshape, Origin, Dir, Distance);
				if (Hit)
				{
					return true;
				}
			}
		}
		return false;
	}
}