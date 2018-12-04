#include "TDScene.h"
#include "TDOctTree.h"
#include "TDRigidDynamic.h"
#include "TDTypes.h"
#include "TDCollisionHandlers.h"
#include "TDPhysics.h"
#include "TDSolver.h"
#include "TDBroadphase.h"
#include "TDActor.h"
#include "Shapes/TDAABB.h"
namespace TD
{
	TDScene::TDScene()
	{
		AcclerationTree = new TDOctTree();
		Broadphase = new TDBroadphase();
	}


	TDScene::~TDScene()
	{
		SafeDelete(AcclerationTree);
		SafeDelete(Broadphase);
	}
#if !BUILD_FULLRELEASE
	void TDScene::DebugRender()
	{
		for (int i = 0; i < SceneActors.size(); i++)
		{
			SceneActors[i]->AABB->DebugRender();
		}
	}
#endif
	void TDScene::AddToScene(TDActor * Actor)
	{
		Actor->Init();
		SceneActors.push_back(Actor);
		Actor->OwningScene = this;
		TDRigidDynamic* Dynamic = TDActor::ActorCast<TDRigidDynamic>(Actor);
		if (Dynamic != nullptr)
		{
			DynamicActors.push_back(Dynamic);
		}
		Broadphase->AddToPhase(Actor);
	}

	void TDScene::AddConstraint(TDConstraint* con)
	{
		Constraints.push_back(con);
	}

	template<class T>
	void RemoveFromVector(std::vector<T*>& vector, T* target)
	{
		for (int i = 0; i < vector.size(); i++)
		{
			if (vector[i] == target)
			{
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
		Broadphase->RemoveFromPhase(Actor);
	}

	bool TDScene::RayCastScene(glm::vec3 Origin, glm::vec3 Dir, float Distance, RaycastData * HitData)
	{
#if !BUILD_FULLRELEASE
		TDPhysics::StartTimer(TDPerfCounters::IntersectionTests);
#endif
		bool result = RayCastSceneInternal(Origin, Dir, Distance, HitData);
#if !BUILD_FULLRELEASE
		TDPhysics::EndTimer(TDPerfCounters::IntersectionTests);
#endif
		return result;
	}

	void TDScene::UpdateBroadPhase()
	{
		for (int i = 0; i < SceneActors.size(); i++)
		{
			Broadphase->UpdateActor(SceneActors[i]);
		}
		Broadphase->ConstructPairs();
	}

	std::vector<CollisionPair>& TDScene::GetPairs()
	{
		return Broadphase->NarrowPhasePairs;
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
				Hit = con(currentshape, Origin, Dir, Distance, HitData);
				if (Hit)
				{
					return true;
				}
			}
		}
		return false;
	}
}