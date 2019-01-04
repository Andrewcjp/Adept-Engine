#pragma once
#include "TDActor.h"
#include "TDTypes.h"

namespace TD
{
	class TDOctTree;
	class TDRigidDynamic;
	struct RaycastData;
	class TDConstraint;
	class TDBroadphase;
	class TDScene
	{
	public:
		TDScene();
		~TDScene();
#if BUILD_DEBUG_RENDER
		void DebugRender();
#endif
		TD_API void AddToScene(TDActor* Actor);
		void AddConstraint(TDConstraint* con);
		std::vector<TDActor*>& GetActors() { return SceneActors; };
		std::vector<TDRigidDynamic*>& GetDynamicActors() { return DynamicActors; };
		std::vector<TDConstraint*>& GetConstraints() { return Constraints; };
		glm::vec3 GetGravity()const { return GravityForce; }
		TD_API void RemoveActor(TDActor* Actor);
		TD_API bool RayCastScene(glm::vec3 Origin, glm::vec3 Dir, float Distance, RaycastData* HitData, TDQuerryFilter* Filter = nullptr);
		void UpdateBroadPhase();
		std::vector<ActorCollisionPair>& GetPairs();
	private:
		bool RayCastSceneInternal(RayCast* ray);
		glm::vec3 GravityForce = glm::vec3(0, -20.0f, 0);
		//todo: remove
		std::vector<TDActor*> SceneActors;
		std::vector<TDRigidDynamic*> DynamicActors;
		std::vector<TDConstraint*> Constraints;
		TDBroadphase* Broadphase = nullptr;
	};

}