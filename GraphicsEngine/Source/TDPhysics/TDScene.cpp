#include "TDScene.h"
#include "Shapes/TDAABB.h"
#include "TDBroadphase.h"
#include "TDCollisionHandlers.h"
#include "TDPhysics.h"
#include "TDRigidDynamic.h"
#include "Utils/MemoryUtils.h"
namespace TD
{
	TDScene::TDScene()
	{
		Broadphase = new TDBroadphase();
		GravityForce = glm::vec3(0, -20.0f, 0);
	}

	TDScene::~TDScene()
	{
		SafeDelete(Broadphase);
		MemoryUtils::DeleteVector(SceneActors);
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
		DebugEnsure(Actor->GetActorType() != TDActorType::Limit);
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

	bool TDScene::RayCastScene(glm::vec3 Origin, glm::vec3 Dir, float Distance, RaycastData* HitData, TDQuerryFilter* Filter /*= nullptr*/)
	{
#if !BUILD_FULLRELEASE
		TDPhysics::StartTimer(TDPerfCounters::IntersectionTests);
#endif
		RayCast ray(Origin, Dir, Distance, HitData, Filter);
		bool result = RayCastSceneInternal(&ray);
#if !BUILD_FULLRELEASE
		TDPhysics::EndTimer(TDPerfCounters::IntersectionTests);
#endif
		return result;
	}

	void TDScene::UpdateBroadPhase()
	{
		for (int i = 0; i < SceneActors.size(); i++)
		{
			SceneActors[i]->UpdateTransfrom();
			Broadphase->UpdateActor(SceneActors[i]);
		}
		Broadphase->ConstructPairs();
	}

	std::vector<ActorCollisionPair>& TDScene::GetPairs()
	{
		return Broadphase->NarrowPhasePairs;
	}

	bool TDScene::RayCastSceneInternal(RayCast* Ray)
	{
		//todo: MultiCast 
		bool Hit = false;
		for (int i = 0; i < SceneActors.size(); i++)
		{
			TDActor* actor = SceneActors[i];
			if (TDIntersectionHandlers::IntersectAABB(actor->AABB, Ray))
			{
				Ray->HitData->Reset();
				for (int j = 0; j < actor->GetAttachedShapes().size(); j++)
				{
					TDShape* currentshape = actor->GetAttachedShapes()[j];
					if (!Ray->PreFilter(actor,currentshape))
					{
						continue;
					}
					DebugEnsure(currentshape);
					IntersectionMethod con = IntersectionMethodTable[currentshape->GetShapeType()];
					DebugEnsure(con);
					Hit = con(currentshape, Ray);
					if (Hit && Ray->PostFilter())
					{
						return true;
					}
				}
			}
		}
		return false;
	}
}